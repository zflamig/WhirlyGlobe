package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.Okio
import java.io.File
import java.io.IOException

class MapTilerTestCase : MaplyTestCase {
    constructor(activity: Activity) : super(activity) {
        setTestName("MapTiler")
        implementation = TestExecutionImplementation.Both
    }

    var loader: QuadPagingLoader? = null
    var polyStyle: VectorStyleSimpleGenerator? = null
    var interp: MapboxVectorInterpreter? = null
//    var lineStyleGen: VectorStyleSimpleGenerator? = null
//    var tileRenderer: RenderController? = null

    // Set up the loader (and all the stuff it needs) for the map tiles
    fun setupLoader(control: BaseController, testType: ConfigOptions.TestType) {
        val assetMgr = getActivity().assets
        val stream = assetMgr.open("maptiler_basic.json")
        var polyStyle: MapboxVectorStyleSet?
        try {
            val json = Okio.buffer(Okio.source(stream)).readUtf8()
            polyStyle = MapboxVectorStyleSet(json, null, control.getActivity().resources.displayMetrics, control)
        }
        catch (e: IOException) {
            return
        }

        // Ya basic, MapTiler
        val cacheDirName = "maptiler-basic"
        val cacheDir = File(getActivity().cacheDir, cacheDirName)
        cacheDir.mkdir()

        val tileInfo = RemoteTileInfoNew("https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=8iZUKgsBTIFhFIZjA5lm",
                0, 14)
        tileInfo.cacheDir = cacheDir

        // Sampling params define how the globe is broken up, including the depth
        var params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.minImportanceTop = 0.0
        params.minImportance = 1024.0 * 1024.0
        params.singleLevel = true
        params.minZoom = tileInfo.minZoom
        params.maxZoom = tileInfo.maxZoom
        if (testType == ConfigOptions.TestType.GlobeTest) {
            params.coverPoles = true
            params.edgeMatching = true
        }

        // Need a standalone renderer
//        tileRenderer = RenderController(512,512)

        // The interpreter renders some of the data into images and overlays the rest
//        interp = MapboxVectorInterpreter(polyStyle, tileRenderer, polyStyle, control)
        interp = MapboxVectorInterpreter(null, null, polyStyle, control)

        // Finally the loader asks for tiles
        loader = QuadPagingLoader(params, tileInfo, interp, control)
//        loader = QuadImageLoader(params,tileInfo,control)
        loader?.setLoaderInterpreter(interp)

        control.setClearColor(polyStyle.backgroundColorForZoom(0.0))
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        setupLoader(globeVC!!, ConfigOptions.TestType.GlobeTest)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        setupLoader(mapVC!!, ConfigOptions.TestType.GlobeTest)

        return true
    }
}