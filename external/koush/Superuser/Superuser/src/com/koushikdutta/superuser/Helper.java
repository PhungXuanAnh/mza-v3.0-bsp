/*
 * Copyright (C) 2013 Koushik Dutta (@koush)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.koushikdutta.superuser;

import java.lang.reflect.Method;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;

import com.koushikdutta.superuser.util.ImageCache;

public class Helper {
    public static Drawable loadPackageIcon(Context context, String pn) {
        try {
            PackageManager pm = context.getPackageManager();
            PackageInfo pi = context.getPackageManager().getPackageInfo(pn, 0);
            Drawable ret = ImageCache.getInstance().get(pn);
            if (ret != null)
                return ret;
            ImageCache.getInstance().put(pn, ret = pi.applicationInfo.loadIcon(pm));
            return ret;
        }
        catch (Exception ex) {
        }
        return null;
    }
}
