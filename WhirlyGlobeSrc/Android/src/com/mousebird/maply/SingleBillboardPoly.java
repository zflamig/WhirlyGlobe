/*
 *  SingleBillboardPoly.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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
package com.mousebird.maply;

import android.graphics.Color;


public class SingleBillboardPoly {

    public SingleBillboardPoly() {
        initialise();
    }

    public void finalize()
    {
        dispose();
    }

    public native void addPoint(Point2d pt);

    public native void addTexCoord(Point2d texCoord);

    public native void addColor (float r, float g, float b, float a);

    public native float[] getColor();

    public native void setTexID(long texID);

    public native long getTexID();

    public native void addVertexAttribute(VertexAttribute attribute);


    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}

