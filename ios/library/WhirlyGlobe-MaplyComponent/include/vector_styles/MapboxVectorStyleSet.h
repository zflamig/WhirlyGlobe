/*
 *  MapboxVectorStyleSet.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/16/15.
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

#import <Foundation/Foundation.h>
#import "vector_styles/MaplyVectorStyle.h"
#import "vector_styles/MapboxVectorStyleSprites.h"
#import "vector_tiles/MapboxVectorTiles.h"

@class MapboxVectorFilter;
@class MaplyVectorFunctionStops;
@class MapboxTransDouble;
@class MapboxTransColor;

/** @brief The Mapbox Vector Style Set parses Mapbox GL Style sheets and turns them into Maply compatible styles.
    @details A style delegate is required by the Mapnik parser to build geometry out of Mapnik vector tiles.  This style delegate can read a Mapbox GL Style sheet and produce compatible styles.
 */
@interface MapboxVectorStyleSet : NSObject <MaplyVectorStyleDelegate>

/// @brief Initialize with the style JSON and the view controller
/// @details We'll parse the style JSON passed in and return nil on failure.
/// @details The optional filter can be used to reject layers we won't use
- (id __nullable)initWithJSON:(NSData * __nonnull)styleJSON settings:(MaplyVectorStyleSettings * __nonnull)settings viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC filter:(bool (^_Nullable)(NSMutableDictionary * __nonnull))filterBlock;

/// @brief Where we can fetch the sprites
@property (nonatomic, strong, nullable) NSString *spriteURL;

/// Tile sources
@property (nonatomic, strong, nonnull) NSArray *sources;

/// If there is a background layer, calculate the color for a given zoom level.
/// Otherwise return nil
- (UIColor * __nullable)backgroundColorForZoom:(double)zoom;

@end

typedef enum : NSUInteger {
    MapboxSourceVector,
    MapboxSourceRaster,
    // TODO: Support the rest of these eventually
} MapboxSourceType;

// Sources are called out individually
@interface MaplyMapboxVectorStyleSource : NSObject

// Name of the source
@property (nonatomic,nullable) NSString *name;

// Vector and raster sources supported for now
@property (nonatomic) MapboxSourceType type;

// TileJSON URL, if present
@property (nonatomic,nullable) NSString *url;

// If the TileJSON spec is inline, this is it
@property (nonatomic,nullable) NSDictionary *tileSpec;

// Initialize with the entry in the style file
- (id __nullable)initWithName:(NSString *__nonnull)name styleEntry:(NSDictionary * __nonnull)styleEntry styleSet:(MapboxVectorStyleSet * __nonnull)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

@end
