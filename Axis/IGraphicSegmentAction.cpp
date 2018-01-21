// Copyright 2018 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Axis.h"

   HRESULT Axis::LeftMouse() {
   if ( selected ) 
      return MenuRequest();
   return S_OK;  
   }
 
 
   HRESULT Axis::RightMouse() {
   if ( ! selected ) 
      return Selector();
   return S_OK;
   }
 
 
   HRESULT Axis::MenuRequest() {
   POINT ptlMouse;
   GetCursorPos(&ptlMouse);
   TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,ptlMouse.x,ptlMouse.y,0,hwndObjectWindow,NULL);
   return 1;
   }
 
 
   HRESULT Axis::Unselector() {
   if ( selected ) 
      return Selector();
   return S_OK;
   }
 
 
   HRESULT Axis::Selector() {
   char szText[MAX_PROPERTY_SIZE];
   sprintf(szText,"Axis[(%lf,%lf,%lf) <-> (%lf,%lf,%lf)]",minPoint.x,minPoint.y,minPoint.z,maxPoint.x,maxPoint.y,maxPoint.z);
   if ( pIGSystemStatusBar )
      pIGSystemStatusBar -> put_StatusText(0,szText);
   return S_OK;
   }
 

   HRESULT Axis::MouseMove(POINT* ptMouse) {
   return S_OK;
   }


   HRESULT Axis::MouseRelease() {
   return S_OK;
   }