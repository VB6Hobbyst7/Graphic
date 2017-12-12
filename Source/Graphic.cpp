/*

                       Copyright (c) 1996,1997,1998,1999,2000,2001,2002,2008,2009 Nathan T. Clark

*/

#include <windows.h>
#include <commctrl.h>

#include "Graphic_resource.h"

#include "utils.h"
#include "ContainedFunction.h"
#include "Graphic.h"

#include "List.cpp"

   WNDPROC G::defaultStatusBarHandler = NULL;
   WNDPROC G::defaultPatchPainter = NULL;

   G::G(IUnknown *pUnkOuter) :

      plotList(),
      functionList(),
      cachedFunctionList(),
      renderThreadList(),
      graphicCursorList(),

      refCount(0),
 
      pIUnknownOuter(pUnkOuter),
 
      pIGProperties(NULL),
      pIOpenGLImplementation(NULL),
      pIEvaluator(NULL),

      pIOleClientSite(NULL),
      pIOleInPlaceSite(NULL),
      pAdviseSink(NULL),
      pOleAdviseHolder(NULL),

      pSelectedGraphicSegmentAction(NULL),
      pIGSystemStatusBar(NULL),

      pDataAdviseHolder(NULL),
 
      pIDataSetMaster(NULL),

      pPlot_IClassFactory(NULL),
      pFunction_IClassFactory(NULL),
      pIViewSet(NULL),

      hwndFrame(NULL),
      hwndGraphic(NULL),
      hwndOwner(NULL),
      hwndStatusBar(NULL),
      hMenuPlot(NULL),
      hwndAppearanceSettings(NULL),
      hwndStyleSettings(NULL),
      hwndTextSettings(NULL),
      hwndLightingSettings(NULL),
      hwndBackgroundSettings(NULL),
      hwndPlotSettings(NULL),
      hwndFunctionSettings(NULL),
      hwndAxisSettings(NULL),
      hwndDataSourcesDialog(NULL),
      hwndDataSourcesTab(NULL),
      hwndDataSourcesFunctions(NULL),

      hitTable(NULL),
      hitTableHits(0),
      rendering(FALSE),
      eraseMode(FALSE),
      windowStorageBytes(128),
      windowStyle(0),
      windowFrameFlags(0),

      autoClear(TRUE),
      autoPlotViewDetection(TRUE),
      showStatusBar(TRUE),
      useGraphicsCursor(TRUE),
      denyUserPropertySettings(FALSE),
      allowUserFunctionControlVisibilityAccess(FALSE),
      isRunning(FALSE),

      showMargins(FALSE),
      showFunctions(FALSE),
      stretchToMargins(FALSE),

      xaxis(0),
      yaxis(0),
      zaxis(0),

      plotView(gcPlotView2D),
      plotType(gcPlotTypeNone),

      plotCount(0),
      textCount(0),
      functionCount(0),
      currentPlotSourceID(0),

      supportedLightCount(0),
      lightPositions(NULL),

      adviseSink_dwAspect(0),
      adviseSink_advf(0),

      xPixelsPerUnit(0),
      yPixelsPerUnit(0),

      zLevel(1.0),
      floorZ(0.0),
      ceilingZ(1.0)
  {

   statusTextWidth[0] = -1;
   statusTextWidth[1] = -1;

   pIPropertiesClient = new _IGPropertiesClient(this);
   pIPropertyPageClient = new _IGPropertyPageClient(this);

   memset(pIPropertyPage,0,sizeof(pIPropertyPage));

   //char szTemp[64];
 
   refCount = 100;
 
   containerSize.cx = PLOT_DEFAULT_WIDTH;
   containerSize.cy = PLOT_DEFAULT_HEIGHT;
 
   memset(szName,0,sizeof(szName));

   memset(windowTitle,0,sizeof(windowTitle));

   ptlMouseBeforeMenu.x = ptlMouseBeforeMenu.y = 0;

   memset(&rightMouseClickPosition,0,sizeof(rightMouseClickPosition));

   memset(&rectStatusText,0,sizeof(rectStatusText));

   memset(plotMarginUnits,0,sizeof(plotMarginUnits));

   margins.left = PLOT_MARGIN_LEFT;
   margins.top = PLOT_MARGIN_TOP;
   margins.right = PLOT_MARGIN_RIGHT;
   margins.bottom = PLOT_MARGIN_BOTTOM;

   pickBoxSize.cx = pickBoxSize.cy = PICK_WINDOW_SIZE;
 
   memset(&ptlZoomAnchor,0,sizeof(POINT));
   memset(&ptlZoomFloat,0,sizeof(POINT));
   memset(&ptlLastMouse,0,sizeof(POINT));
   ptlPickBox.x = -1L;
   ptlPickBox.y = -1L;

   trackedMouse = FALSE;

   QueryInterface(IID_IUnknown,reinterpret_cast<void**>(&pIUnknownThis));
 
   QueryInterface(IID_IGSystemStatusBar,reinterpret_cast<void**>(&pIGSystemStatusBar));
   pIGSystemStatusBar -> Release();

   CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));
 
   CoGetClassObject(CLSID_Plot,CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,NULL,IID_IClassFactory,reinterpret_cast<void **>(&pPlot_IClassFactory));

   CoGetClassObject(CLSID_GSystemFunctioNater,CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,NULL,IID_IClassFactory,reinterpret_cast<void **>(&pFunction_IClassFactory));

   CoCreateInstance(CLSID_DataSet,pIUnknownOuter,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IDataSet,reinterpret_cast<void **>(&pIDataSetMaster));

   CoCreateInstance(CLSID_OpenGLImplementor,pIUnknownThis,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IOpenGLImplementation,reinterpret_cast<void **>(&pIOpenGLImplementation));

   CoCreateInstance(CLSID_Evaluator,pIUnknownThis,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IEvaluator,reinterpret_cast<void **>(&pIEvaluator));

   pIOpenGLImplementation -> Initialize(pIEvaluator);

   pIOpenGLImplementation -> AdviseGSystemStatusBar(pIGSystemStatusBar);

   pIGProperties -> Advise(pIPropertiesClient);

   pIGProperties -> AdvisePropertyPageClient(pIPropertyPageClient,true);

   pIGProperties -> put_DebuggingEnabled(true);

   pIDataSetMaster -> ReSet();
 
   pIGProperties -> Add(L"auto clear",&propertyAutoClear);

   pIGProperties -> Add(L"plot view",&propertyPlotView);
   pIGProperties -> Add(L"plot type",&propertyPlotType);
   pIGProperties -> Add(L"auto plotView detection",&propertyAutoPlotViewDetection);

   pIGProperties -> Add(L"plot width",NULL);
   pIGProperties -> Add(L"plot height",NULL);

   pIGProperties -> Add(L"plot margin units",&propertyPlotMarginUnits);
   pIGProperties -> Add(L"plot left margin",&propertyPlotLeftMargin);
   pIGProperties -> Add(L"plot right margin",&propertyPlotRightMargin);
   pIGProperties -> Add(L"plot top margin",&propertyPlotTopMargin);
   pIGProperties -> Add(L"plot bottom margin",&propertyPlotBottomMargin);
   pIGProperties -> Add(L"plot margins",&propertyPlotMargins);
   pIGProperties -> Add(L"plot margins stretch all",&propertyPlotMarginsStretchAll);
                                                                                                                                                                                    
   pIGProperties -> Add(L"plot count",NULL);                                                                                                                            
   pIGProperties -> Add(L"text count",NULL);
   pIGProperties -> Add(L"function count",NULL);
                                                                                                                                                                                    
   pIGProperties -> Add(L"floor z",&propertyFloor);
   pIGProperties -> Add(L"ceiling z",&propertyCeiling);

   IGProperty* pTemp;
   pIGProperties -> Add(L"pick box size",&pTemp);
   pTemp -> put_type(TYPE_BINARY);
   pTemp -> put_size(sizeof(SIZEL));

   pIGProperties -> Add(L"print properties",&propertyPrintProperties);
   propertyPrintProperties -> put_type(TYPE_BINARY);
   propertyPrintProperties -> put_size(sizeof(PRINTDLG));

   pIGProperties -> Add(L"data extents",&propertyDataExtents);
   propertyDataExtents -> put_type(TYPE_BINARY);
   propertyDataExtents -> put_size(128);

   pIGProperties -> Add(L"background color",&propertyBackgroundColor);
   propertyBackgroundColor -> put_type(TYPE_BINARY);
   propertyBackgroundColor -> put_size(4 * sizeof(float));

   pIGProperties -> Add(L"text color",&propertyTextColor);
   propertyTextColor -> put_type(TYPE_BINARY);
   propertyTextColor -> put_size(4 * sizeof(float));

   pIGProperties -> Add(L"text background color",&propertyTextBackgroundColor);
   propertyTextBackgroundColor -> put_type(TYPE_BINARY);
   propertyTextBackgroundColor -> put_size(4 * sizeof(float));

   pIGProperties -> Add(L"custom colors",&propertyCustomColors);
   propertyCustomColors -> put_type(TYPE_BINARY);
   propertyCustomColors -> put_size(128);

   pIGProperties -> Add(L"view phi",&propertyViewPhi);
   pIGProperties -> Add(L"view theta",&propertyViewTheta);
   pIGProperties -> Add(L"view spin",&propertyViewSpin);

   pIGProperties -> Add(L"deny user property settings",&propertyDenyUserPropertySettings);

   pIGProperties -> Add(L"light count",&propertyCountLights);

   pIGProperties -> Add(L"specular reference",&propertySpecularReference);
   propertySpecularReference -> put_type(TYPE_BINARY);
   propertySpecularReference -> put_size(4 * sizeof(float));

   pIGProperties -> Add(L"ambient reference",&propertyAmbientReference);
   propertyAmbientReference -> put_type(TYPE_BINARY);
   propertyAmbientReference -> put_size(4 * sizeof(float));

   pIGProperties -> Add(L"shinyness",&propertyShinyness);

   pIOpenGLImplementation -> get_SupportedLightCount(&supportedLightCount);

   ppPropertyLightOn = new IGProperty*[supportedLightCount];
   ppPropertyAmbientLight = new IGProperty*[supportedLightCount];
   ppPropertyDiffuseLight = new IGProperty*[supportedLightCount];
   ppPropertySpecularLight = new IGProperty*[supportedLightCount];
   ppPropertyLightPos = new IGProperty*[supportedLightCount];

   lightPositions = new char**[supportedLightCount];
   memset(lightPositions,0,supportedLightCount * sizeof(char**));

   WCHAR szwTemp[64];

   for ( long k = 0; k < supportedLightCount; k ++ ) {

      swprintf(szwTemp,L"light %ld enabled",k);  
      pIGProperties -> Add(szwTemp,&ppPropertyLightOn[k]);

      swprintf(szwTemp,L"ambient light %ld",k);  
      pIGProperties -> Add(szwTemp,&ppPropertyAmbientLight[k]);

      swprintf(szwTemp,L"diffuse light %ld",k);  
      pIGProperties -> Add(szwTemp,&ppPropertyDiffuseLight[k]);

      swprintf(szwTemp,L"specular light %ld",k); 
      pIGProperties -> Add(szwTemp,&ppPropertySpecularLight[k]);

      swprintf(szwTemp,L"light position %ld",k); 
      pIGProperties -> Add(szwTemp,&ppPropertyLightPos[k]);

   }

   pIGProperties -> Add(L"functions",&propertyFunctions);
   pIGProperties -> Add(L"show functions",&propertyShowFunctions);
   pIGProperties -> Add(L"allow user ability to access function control visibility",&propertyAllowUserFunctionControlVisibilityAccess);
   pIGProperties -> Add(L"plots",&propertyPlots);
   pIGProperties -> Add(L"texts",&propertyTexts);
   pIGProperties -> Add(L"axiis",&propertyAxiis);
   propertyAxiis -> put_type(TYPE_OBJECT_STORAGE_ARRAY);

   pIGProperties -> Add(L"show status bar",&propertyShowStatusBar);
   pIGProperties -> Add(L"status text",&propertyStatusText);
   propertyStatusText -> put_type(TYPE_SZSTRING);
   propertyStatusText -> put_szValue("");
   pIGProperties -> Add(L"show margins",&propertyShowMargins);
   
   pIGProperties -> Add(L"use graphics cursor",&propertyUseGraphicsCursor);

   pIGProperties -> Add(L"opengl axis text",&propertyRenderOpenGLAxisText);
   propertyRenderOpenGLAxisText -> put_type(TYPE_BOOL);
   propertyRenderOpenGLAxisText -> put_size(sizeof(BOOL));
   propertyRenderOpenGLAxisText -> put_boolValue(FALSE);
   
   pIGProperties -> DirectAccess(L"auto clear",TYPE_BOOL,&autoClear,sizeof(autoClear));
   pIGProperties -> DirectAccess(L"plot view",TYPE_LONG,&plotView,sizeof(plotView));
   pIGProperties -> DirectAccess(L"plot type",TYPE_LONG,&plotType,sizeof(plotType));
   pIGProperties -> DirectAccess(L"plot width",TYPE_LONG,&containerSize.cx,sizeof(containerSize.cx));
   pIGProperties -> DirectAccess(L"plot height",TYPE_LONG,&containerSize.cy,sizeof(containerSize.cy));
   pIGProperties -> DirectAccess(L"plot count",TYPE_LONG,&plotCount,sizeof(plotCount));
   pIGProperties -> DirectAccess(L"text count",TYPE_LONG,&textCount,sizeof(textCount));
   pIGProperties -> DirectAccess(L"function count",TYPE_LONG,&functionCount,sizeof(functionCount));
   pIGProperties -> DirectAccess(L"pick box size",TYPE_BINARY,&pickBoxSize,sizeof(pickBoxSize));
   pIGProperties -> DirectAccess(L"floor z",TYPE_DOUBLE,&floorZ,sizeof(floorZ));
   pIGProperties -> DirectAccess(L"ceiling z",TYPE_DOUBLE,&ceilingZ,sizeof(ceilingZ));
   pIGProperties -> DirectAccess(L"show functions",TYPE_BOOL,&showFunctions,sizeof(showFunctions));

   propertyDenyUserPropertySettings -> directAccess(TYPE_BOOL,&denyUserPropertySettings,sizeof(denyUserPropertySettings));
   propertyAllowUserFunctionControlVisibilityAccess -> directAccess(TYPE_BOOL,&allowUserFunctionControlVisibilityAccess,sizeof(allowUserFunctionControlVisibilityAccess));

   propertyShowStatusBar -> directAccess(TYPE_BOOL,&showStatusBar,sizeof(showStatusBar));
   propertyAutoPlotViewDetection -> directAccess(TYPE_BOOL,&autoPlotViewDetection,sizeof(autoPlotViewDetection));
   propertyUseGraphicsCursor -> directAccess(TYPE_BOOL,&useGraphicsCursor,sizeof(useGraphicsCursor));
   propertyShowMargins -> directAccess(TYPE_BOOL,&showMargins,sizeof(showMargins));

   propertyPlotMarginUnits -> directAccess(TYPE_SZSTRING,plotMarginUnits,sizeof(plotMarginUnits));
   propertyPlotLeftMargin -> directAccess(TYPE_LONG,&margins.left,sizeof(margins.left));
   propertyPlotTopMargin -> directAccess(TYPE_LONG,&margins.top,sizeof(margins.top));
   propertyPlotRightMargin -> directAccess(TYPE_LONG,&margins.right,sizeof(margins.right));
   propertyPlotBottomMargin -> directAccess(TYPE_LONG,&margins.bottom,sizeof(margins.bottom));
   propertyPlotMargins -> directAccess(TYPE_BINARY,&margins,sizeof(RECT));
   propertyPlotMarginsStretchAll -> directAccess(TYPE_BOOL,&stretchToMargins,sizeof(BOOL));

   CoCreateInstance(CLSID_GSystemAxis,pIUnknownOuter,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IAxis,reinterpret_cast<void **>(&xaxis));
   CoCreateInstance(CLSID_GSystemAxis,pIUnknownOuter,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IAxis,reinterpret_cast<void **>(&yaxis));
   CoCreateInstance(CLSID_GSystemAxis,pIUnknownOuter,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IAxis,reinterpret_cast<void **>(&zaxis));

   xaxis -> AdviseGSystemStatusBar(pIGSystemStatusBar);
   yaxis -> AdviseGSystemStatusBar(pIGSystemStatusBar);
   zaxis -> AdviseGSystemStatusBar(pIGSystemStatusBar);

   axisList.Add(xaxis);
   axisList.Add(yaxis);
   axisList.Add(zaxis);

   HRESULT rc = CoCreateInstance(CLSID_ViewSet,pIUnknownOuter,CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,IID_IViewSet,reinterpret_cast<void **>(&pIViewSet));

   IUnknown* pIViewSet_Unknown;
   pIViewSet -> QueryInterface(IID_IUnknown,reinterpret_cast<void**>(&pIViewSet_Unknown));
   pIGProperties -> AddPropertyPage(pIViewSet_Unknown);
   pIViewSet_Unknown -> Release();

   pIPropertiesClient -> InitNew();

   refCount = 0;

   pIPropertyPage[0] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesPosSize);
   pIPropertyPage[1] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesStyle);
   pIPropertyPage[2] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesBackground);
   pIPropertyPage[3] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesText);
   pIPropertyPage[4] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesLighting);
   pIPropertyPage[5] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesAxis);
   pIPropertyPage[6] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesPlot);
   pIPropertyPage[7] = new _IPropertyPage(this,CLSID_GSystemGraphicPropertiesFunctions);

   return;
   }
 
 
   G::~G() {
 
   stop();

   delete [] ppPropertyLightOn;
   delete [] ppPropertyAmbientLight;
   delete [] ppPropertyDiffuseLight;
   delete [] ppPropertySpecularLight;
   delete [] ppPropertyLightPos;

   IPlot *p;
   while ( p = plotList.GetFirst() ) { 
      p -> Release();
      plotList.Remove(p); 
   }

   IGSFunctioNater *pf;
   while ( pf = functionList.GetFirst() ) { 
      ContainedFunction* pContainedFunction = containedFunctionList.Get(reinterpret_cast<long>(pf));
      if ( pContainedFunction ) {
         IOleObject *pIOleObject;
         pf -> QueryInterface(IID_IOleObject,reinterpret_cast<void**>(&pIOleObject));
         pIOleObject -> SetClientSite(NULL);
         pf -> Release();
         containedFunctionList.Remove(pContainedFunction);
         delete pContainedFunction;
      }
      pf -> Release();
      functionList.Remove(pf); 
   }

   while ( pf = cachedFunctionList.GetFirst() ) {
      pf -> Release();
      cachedFunctionList.Remove(pf);
   }

   IText *t;
   while ( t = textList.GetFirst() ) {
      t -> Release();
      textList.Remove(t);
   }
   textList.Delete();
 
   if ( pIViewSet )
      pIViewSet -> Release();

   if ( xaxis ) xaxis -> Release();
   if ( yaxis ) yaxis -> Release();
   if ( zaxis ) zaxis -> Release();

   pIDataSetMaster -> Release();

   if ( pIOpenGLImplementation ) {
      pIOpenGLImplementation -> UnSetViewProperties();
      pIOpenGLImplementation -> Stop();      
      pIOpenGLImplementation -> Release();
   }
 
   SetParent(hwndFrame,NULL);
   DestroyWindow(hwndFrame);

   pIGProperties -> Release();

   pIPropertiesClient -> Release();

   delete pIPropertiesClient;
   delete pIPropertyPageClient;

   if ( pIOleInPlaceSite ) 
      pIOleInPlaceSite -> Release();

// The following statement causes too early of a release (there is an additional AddRef() somewhere )
//??   if ( pIOleClientSite ) 
//??      pIOleClientSite -> Release();

   pIOleInPlaceSite = NULL;
   pIOleClientSite = NULL;

   DestroyMenu(hMenuPlot);

   delete pIPropertyPage[0];
   delete pIPropertyPage[1];
   delete pIPropertyPage[2];
   delete pIPropertyPage[3];
   delete pIPropertyPage[4];
   delete pIPropertyPage[5];
   delete pIPropertyPage[6];
   delete pIPropertyPage[7];

   return;
   }
 
 
   void G::changeType() {
   MENUITEMINFO mi;
   memset(&mi,0,sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_STATE;
   mi.fState = MFS_UNCHECKED;

   SetMenuItemInfo(hwndMenuPlot(),IDMI_GRAPHIC_VIEW_2D,MF_BYCOMMAND,&mi);
   SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_SURFACE,MF_BYCOMMAND,&mi);
   SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_WIREFRAME,MF_BYCOMMAND,&mi);
   SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_NATURAL,MF_BYCOMMAND,&mi);
   SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_STACKS,MF_BYCOMMAND,&mi);

   mi.fState = MFS_CHECKED;

   switch ( plotView ) {

   case gcPlotView2D:
      SetMenuItemInfo(hwndMenuPlot(),IDMI_GRAPHIC_VIEW_2D,MF_BYCOMMAND,&mi);
      break;
 
   case gcPlotView3D:

      SetMenuItemInfo(hwndMenuPlot(),IDMI_GRAPHIC_VIEW_3D,MF_BYCOMMAND,&mi);

      switch ( plotType ) {
      case gcPlotTypeSurface:
         SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_SURFACE,MF_BYCOMMAND,&mi);
         break;
 
      case gcPlotTypeWireFrame:
         SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_WIREFRAME,MF_BYCOMMAND,&mi);
         break;
 
      case gcPlotTypeNatural:
         SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_NATURAL,MF_BYCOMMAND,&mi);
         break;
 
      case gcPlotTypeStacks:
         SetMenuItemInfo(hwndMenuPlot(),IDDI_GRAPHIC_SUB_STYLE_STACKS,MF_BYCOMMAND,&mi);
         break;
 
      }
 
      break;
   }
   return;
   }
 
 
   IGSFunctioNater* G::newFunction(bool deferVisibility) {

   long functionID = functionList.Count() + 1;

   IGSFunctioNater *pIFunction = (IGSFunctioNater*)NULL;

   pFunction_IClassFactory -> CreateInstance(pIUnknownOuter,IID_IGSFunctioNater,reinterpret_cast<void **>(&pIFunction));

   functionList.Add(pIFunction,NULL,functionID);

   if ( hwndFrame ) {

      connectFunction(pIFunction,deferVisibility,false);

      //pIFunction -> put_ShowExpression(TRUE);
      //pIFunction -> put_ShowControls(TRUE);
      pIFunction -> put_AllowControlVisibilitySettings(allowUserFunctionControlVisibilityAccess);
      //pIFunction -> put_ShowResults(TRUE);
      //pIFunction -> put_ShowVariables(TRUE);
      //pIFunction -> put_AllowPropertySettings(TRUE);

      if ( ! deferVisibility ) {
         RECT rect;  
         GetWindowRect(hwndFrame,&rect);
         SendMessage(hwndFrame,WM_SIZE,(WPARAM)SIZE_RESTORED,MAKELPARAM(rect.right - rect.left,rect.bottom - rect.top));
      }

   } else {

      cachedFunctionList.Add(pIFunction,NULL,functionID);
      pIFunction -> AddRef();

   }

   pIFunction -> put_OnChangeCallback(someObjectChanged,(void *)this);

   return pIFunction;
   }


   void G::deleteFunction(IGSFunctioNater *pIFunction) {

   unConnectFunction(pIFunction);

   ContainedFunction* pContainedFunction = containedFunctionList.Get(reinterpret_cast<long>(pIFunction));

   containedFunctionList.Remove(pContainedFunction);

   int item = functionList.IndexOf(pIFunction);

   boolean isCurrentSelection = (item == (int)SendMessage(hwndDataSourcesFunctions,TCM_GETCURSEL,0L,0L));

   functionList.Remove(pIFunction);

   VARIANT_BOOL anyControlVisible;

   pIFunction -> get_AnyControlVisible(&anyControlVisible);

   pIFunction -> Release();

   delete pContainedFunction;

   if ( anyControlVisible ) {

      SendMessage(hwndDataSourcesFunctions,TCM_DELETEITEM,(WPARAM)item,0L);
      int n = functionList.Count();
      if ( n == 0 ) {
         RECT rect;  
         GetWindowRect(hwndFrame,&rect);
         SendMessage(hwndFrame,WM_SIZE,(WPARAM)SIZE_RESTORED,MAKELPARAM(rect.right - rect.left,rect.bottom - rect.top));
         return;
      }

      TC_ITEM tcItem;
      char szTemp[32];
      memset(&tcItem,0,sizeof(tcItem));
      tcItem.mask = TCIF_TEXT;
      tcItem.pszText = szTemp;
      for ( int k = 0; k < n; k++ ) {
         sprintf(szTemp,"F%ld",k);
         SendMessage(hwndDataSourcesFunctions,TCM_SETITEM,(WPARAM)k,(LPARAM)&tcItem);
      }

   }

   if ( 0 < containedFunctionList.Count() ) {
      if ( isCurrentSelection ) {
         SendMessage(hwndDataSourcesFunctions,TCM_SETCURSEL,0L,0L);
         pContainedFunction = containedFunctionList.GetFirst();
         pContainedFunction -> pFunction() -> get_AnyControlVisible(&anyControlVisible);
         if ( anyControlVisible )
            ShowWindow(pContainedFunction -> HWNDSite(),SW_SHOW);
      } else {
         InvalidateRect(hwndDataSourcesFunctions,NULL,TRUE);
      }
   }

   return;
   }


   int G::connectFunction(IGSFunctioNater *pIFunction,bool deferVisibility,bool doVisibilityOnly) {

   int item;

   if ( ! deferVisibility || doVisibilityOnly ) {

      TC_ITEM tie;
      char szFunction[32];

      sprintf(szFunction,"F%d",functionList.IndexOf(pIFunction) + 1);

      memset(&tie,0,sizeof(TC_ITEM));

      tie.mask = TCIF_TEXT;
      tie.pszText = szFunction;
      tie.cchTextMax = 32;

      item = (int)SendMessage(hwndDataSourcesFunctions,TCM_INSERTITEM,(WPARAM)SendMessage(hwndDataSourcesFunctions,TCM_GETITEMCOUNT,0L,0L),(LPARAM)&tie);

   }

   ContainedFunction* pContainedFunction = NULL;

   if ( ! doVisibilityOnly ) {
      IUnknown* pIUnknownFunction;
      pIFunction -> QueryInterface(IID_IUnknown,reinterpret_cast<void**>(&pIUnknownFunction));
      pContainedFunction = new ContainedFunction(this,functionList.ID(pIFunction),hwndDataSourcesFunctions,pIFunction,pIUnknownFunction,DIID_IGSFunctioNaterEvents);
      pIUnknownFunction -> Release();
   } else {
      if ( ! deferVisibility ) {
         while ( pContainedFunction = containedFunctionList.GetNext(pContainedFunction) ) {
            if ( pContainedFunction -> pFunction() == pIFunction ) {
               TC_ITEM tie = {0};
               tie.mask = TCIF_PARAM;
               tie.lParam = (LPARAM)pContainedFunction -> HWNDSite();
               SendMessage(hwndDataSourcesFunctions,TCM_SETITEM,item,(LPARAM)&tie);
               return 1;
            }
         }
      }
   }

   if ( doVisibilityOnly )
      return 1;

   IOleObject* pIOleObject;
   IOleClientSite* pIOleClientSite;

   pIFunction -> QueryInterface(IID_IOleObject,reinterpret_cast<void**>(&pIOleObject));

   pContainedFunction -> QueryInterface(IID_IOleClientSite,reinterpret_cast<void**>(&pIOleClientSite));

   pIOleObject -> SetClientSite(pIOleClientSite);

   pContainedFunction -> connectEvents();

   containedFunctionList.Add(pContainedFunction,NULL,(long)pIFunction);

   pIOleObject -> Release();

   pIOleClientSite -> Release();

   return 1;
   }


   int G::unConnectFunction(IGSFunctioNater* pIFunction) {
   IOleObject* pIOleObject;
   pIFunction -> QueryInterface(IID_IOleObject,reinterpret_cast<void**>(&pIOleObject));
   pIOleObject -> SetClientSite(NULL);
   pIOleObject -> Release();
   return 0;
   }


   IText* G::newText() {
   IText* pIText;
   CoCreateInstance(CLSID_Text,
                    pIUnknownOuter,
                    CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                    IID_IText,
                    reinterpret_cast<void **>(&pIText));

// Possible Release() problem !!!

   ITextNotify *pITextNotify;
   QueryInterface(IID_ITextNotify,reinterpret_cast<void **>(&pITextNotify));
   pIText -> put_TextNotify(pITextNotify);
   pITextNotify -> Release();

   pIText -> put_PartOfWorldDomain(FALSE);

   pIText -> Initialize(hwndGraphic,pIOpenGLImplementation,pIEvaluator,pIDataSetMaster,propertyFloor,propertyCeiling,propertyRenderOpenGLAxisText,NULL,NULL,someObjectChanged,(void *)this);

   pIText -> AdviseGSystemStatusBar(pIGSystemStatusBar);

   textList.Add(pIText);

   return pIText;
   }

 
   IPlot* G::newPlot(long plotID) {

   IPlot* pIPlot;

   pPlot_IClassFactory -> CreateInstance(pIUnknownOuter,IID_IPlot,reinterpret_cast<void **>(&pIPlot));

   pIPlot -> Initialize(pIDataSetMaster,pIOpenGLImplementation,pIEvaluator,NULL,NULL,
                           propertyPlotView,
                           propertyPlotType,
                           propertyBackgroundColor,
                           propertyViewTheta,
                           propertyViewPhi,
                           propertyViewSpin,
                           propertyFloor,
                           propertyCeiling,
                           propertyCountLights,
                           ppPropertyLightOn,
                           ppPropertyAmbientLight,
                           ppPropertyDiffuseLight,
                           ppPropertySpecularLight,
                           ppPropertyLightPos,someObjectChanged,(void *)this);
 
   IPlotNotify *pIPlotNotify;
   QueryInterface(IID_IPlotNotify,reinterpret_cast<void **>(&pIPlotNotify));
   pIPlot -> put_PlotNotify(pIPlotNotify);
   pIPlotNotify -> Release();
   pIPlotNotify -> Release();

   IAdviseSink *pIAdviseSink;
   IViewObjectEx *pIViewObjectEx;

   QueryInterface(IID_IAdviseSink,reinterpret_cast<void **>(&pIAdviseSink));
   pIPlot -> QueryInterface(IID_IViewObjectEx,reinterpret_cast<void **>(&pIViewObjectEx));
   pIViewObjectEx -> SetAdvise(0,0,pIAdviseSink);

   pIViewObjectEx -> Release();
   pIAdviseSink -> Release();

   plotList.Add(pIPlot,NULL,plotID);
 
   pIPlot -> AdviseGSystemStatusBar(pIGSystemStatusBar);

   pIPlot -> put_ParentWindow(hwndGraphic);

   pIPlot -> put_PlotViewProperty(propertyPlotView);

   pIPlot -> put_PlotTypeProperty(propertyPlotType);

   return pIPlot;
   }


   void G::clearData() {
   IPlot* pIPlot = (IPlot *)NULL;
   while ( pIPlot = plotList.GetNext(pIPlot) )
      pIPlot -> ResetData();
   pIDataSetMaster -> ReSet();
   }


   int G::statusPosition() {
   POINT ptMouse;
   double minZ,maxZ;
   pIDataSetMaster -> get_minZ(&minZ);
   pIDataSetMaster -> get_maxZ(&maxZ);
   GetCursorPos(&ptMouse);
   DataPoint dpNewPosition,dpMouse = {(double)ptMouse.x,(double)ptMouse.y,1.0};
   pIOpenGLImplementation -> WindowToData(&dpMouse,&dpNewPosition);
   char szText[128];
   sprintf(szText,"(%lf,%lf,%lf)",dpNewPosition.x,dpNewPosition.y,dpNewPosition.z);
   if ( pIGSystemStatusBar )
      pIGSystemStatusBar -> put_StatusText(1,szText);
   return 0;
   }


   int G::eraseBackground() {

   float fv[4];
   BYTE *pb = (BYTE *)fv;

   propertyBackgroundColor -> get_binaryValue(sizeof(fv),(BYTE **)&pb);

   BYTE vb[3];
   vb[0] = (BYTE)(255.0f*fv[0]);
   vb[1] = (BYTE)(255.0f*fv[1]);
   vb[2] = (BYTE)(255.0f*fv[2]);

   HDC hdc = GetDC(hwndGraphic);

   HBRUSH hb = CreateSolidBrush(RGB(vb[0],vb[1],vb[2]));

   RECT rc = {0};
   GetClientRect(hwndGraphic,&rc);

   SelectObject(hdc,hb);

   FillRect(hdc,&rc,hb);

   DeleteObject(hb);

   ReleaseDC(hwndGraphic,hdc);

   return 0;
   }

   int G::getSegments(long **pSegments) {

   *pSegments = NULL;

   long cntSegments = 0;
   long k;
   IAxis *ia = reinterpret_cast<IAxis *>(NULL);
   IPlot *ap = reinterpret_cast<IPlot *>(NULL);
   IText *it = reinterpret_cast<IText *>(NULL);

   while ( ia = axisList.GetNext(ia) ) {
      ia -> get_SegmentCount(&k);
      cntSegments += k;
   }

   while ( ap = plotList.GetNext(ap) ) {
      ap -> get_SegmentCount(&k);
      cntSegments += k;
   }

   while ( it = textList.GetNext(it) ) {
      it -> get_SegmentCount(&k);
      cntSegments += k;
   }

   if ( ! cntSegments ) 
      return 0;

   *pSegments = new long[cntSegments + 1];
   memset(*pSegments,0,(cntSegments + 1) * sizeof(long));
   long *pLong = *pSegments;

   k = 0;
   while ( ia = axisList.GetNext(ia) ) {
      ia -> get_SegmentCount(&k);
      ia -> GetSegments(pLong);
      pLong += k;
   }

   while ( ap = plotList.GetNext(ap) ) {
      ap -> get_SegmentCount(&k);
      ap -> GetSegments(pLong);
      pLong += k;
   }

   while ( it = textList.GetNext(it) ) {
      it -> get_SegmentCount(&k);
      it -> GetSegments(pLong);
      pLong += k;
   }

   return cntSegments;
   }