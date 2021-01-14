/*
*  MapboxVectorStyleCircle.h
*  WhirlyGlobe-MaplyComponent
*
*  Created by Steve Gifford on 2/17/15.
*  Copyright 2011-2015 mousebird consulting
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*/

#import "MapboxVectorStyleCircle.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

bool MapboxVectorCirclePaint::parse(PlatformThreadInfo *inst,
                                    MapboxVectorStyleSetImpl *styleSet,
                                    DictionaryRef styleEntry)
{
    if (!styleSet)
        return false;
    
    radius = styleSet->transDouble("circle-radius", styleEntry, 5.0);
    fillColor = styleSet->colorValue("circle-color",NULL,styleEntry,RGBAColor::black(),false);
    opacity = styleSet->transDouble("circle-opacity",styleEntry,1.0);
    strokeWidth = styleSet->transDouble("circle-stroke-width",styleEntry,0.0);
    strokeColor = styleSet->colorValue("circle-stroke-color",NULL,styleEntry,RGBAColor::black(),false);
    strokeOpacity = styleSet->transDouble("circle-stroke-opacity",styleEntry,1.0);
    
    return true;
}

bool MapboxVectorLayerCircle::parse(PlatformThreadInfo *inst,
                                    const DictionaryRef &styleEntry,
                                    const MapboxVectorStyleLayerRef &refLayer,
                                    int drawPriority)
{
    if (!styleEntry ||
        !MapboxVectorStyleLayer::parse(inst,styleEntry,refLayer,drawPriority) ||
        !paint.parse(inst, styleSet, styleEntry->getDict("paint")))
    {
        return false;
    }

    const RGBAColor theFillColor = *paint.fillColor;
    const RGBAColor theStrokeColor = *paint.strokeColor;
    const double maxRadius = paint.radius->maxVal();
    const double maxStrokeWidth = paint.strokeWidth->maxVal();
    circleTexID = styleSet->makeCircleTexture(inst,maxRadius,theFillColor,theStrokeColor,maxStrokeWidth,&circleSize);

    // Larger circles are slightly more important
    importance = drawPriority/1000 + styleSet->tileStyleSettings->markerImportance + maxRadius / 100000.0;

    uuidField = styleSet->tileStyleSettings->uuidField;

    return true;
}

void MapboxVectorLayerCircle::cleanup(PlatformThreadInfo *inst,ChangeSet &changes)
{
    if (circleTexID != EmptyIdentity)
    {
        changes.push_back(new RemTextureReq(circleTexID));
    }
}

void MapboxVectorLayerCircle::buildObjects(PlatformThreadInfo *inst,
                                           std::vector<VectorObjectRef> &vecObjs,
                                           const VectorTileDataRef &tileInfo,
                                           const Dictionary *desc)
{
    if (!visible)
        return;

    using MarkerPtrVec = std::vector<WhirlyKit::Marker*>;
    using VecObjRefVec = std::vector<VectorObjectRef>;
    auto const capacity = vecObjs.size() * 5;  // ?
    std::unordered_map<std::string,std::pair<MarkerPtrVec,VecObjRefVec>> markersByUUID(capacity);

    const double opacity = paint.opacity->valForZoom(tileInfo->ident.level);
    const double radius = paint.radius->valForZoom(tileInfo->ident.level);

    // Default settings
    MarkerInfo markerInfo(/*screenObject=*/true);
    markerInfo.zoomSlot = styleSet->zoomSlot;
    markerInfo.color = RGBAColor(255,255,255,opacity*255);
    markerInfo.drawPriority = drawPriority;
    markerInfo.programID = styleSet->screenMarkerProgramID;
    
    if (minzoom != 0 || maxzoom < 1000)
    {
        markerInfo.minZoomVis = minzoom;
        markerInfo.maxZoomVis = maxzoom;
    }

    std::vector<std::unique_ptr<WhirlyKit::Marker>> markerOwner; // automatic cleanup of local temporary allocations
    for (const auto &vecObj : vecObjs)
    {
        if (vecObj->getVectorType() != VectorPointType)
        {
            continue;
        }

        const auto attrs = vecObj->getAttributes();

        for (const VectorShapeRef &shape : vecObj->shapes)
        {
            const auto pts = dynamic_cast<VectorPoints*>(shape.get());
            if (!pts)
            {
                continue;
            }

            for (const auto &pt : pts->pts)
            {
                // Add a marker per point
                markerOwner.emplace_back(std::make_unique<WhirlyKit::Marker>());
                auto marker = markerOwner.back().get();
                marker->loc = GeoCoord(pt.x(),pt.y());
                marker->texIDs.push_back(circleTexID);
                marker->width = 2*radius * styleSet->tileStyleSettings->markerScale;
                marker->height = 2*radius * styleSet->tileStyleSettings->markerScale;
                marker->layoutWidth = marker->width;
                marker->layoutHeight = marker->height;
                marker->layoutImportance = MAXFLOAT;    //importance + (101-tileInfo->ident.level)/100.0;
                marker->uniqueID = uuidField.empty() ? std::string() : attrs->getString(uuidField);

                // Look up the vectors of markers/objects for this uuid (including blank), inserting empty ones if necessary
                const auto result = markersByUUID.insert(std::make_pair(std::ref(marker->uniqueID),
                                                                        std::make_pair(MarkerPtrVec(),
                                                                                       VecObjRefVec())));

                auto &markers = result.first->second.first;
                auto &vecObjs = result.first->second.second;

                if (markers.empty())
                {
                    markers.reserve(pts->pts.size());
                    vecObjs.reserve(pts->pts.size());
                }
                markers.push_back(marker);
                vecObjs.push_back(vecObj);
            }
        }
    }

    for (const auto &kvp : markersByUUID)
    {
        const auto &uuid = kvp.first;
        const auto &markers = kvp.second.first;
        const auto &vecObjs = kvp.second.second;

        // Generate one component object per unique UUID (including blank)
        const auto compObj = styleSet->makeComponentObject(inst, desc);

        compObj->uuid = uuid;

        // Keep the vector objects around if they need to be selectable
        if (selectable)
        {
            assert(markers.size() == vecObjs.size());
            const auto count = std::min(markers.size(), vecObjs.size());
            for (auto i = (size_t)0; i < count; ++i)
            {
                auto *marker = markers[i];
                const auto &vecObj = vecObjs[i];

                marker->isSelectable = true;
                marker->selectID = Identifiable::genId();
                styleSet->addSelectionObject(marker->selectID, vecObj, compObj);
                compObj->selectIDs.insert(marker->selectID);
                compObj->isSelectable = true;
            }
        }

        if (!markers.empty())
        {
            // Set up the markers and get a change set
            if (const auto markerID = styleSet->markerManage->addMarkers(markers, markerInfo, tileInfo->changes))
            {
                compObj->markerIDs.insert(markerID);
            }
            
            styleSet->compManage->addComponentObject(compObj, tileInfo->changes);
            tileInfo->compObjs.push_back(compObj);
        }
    }
}

}
