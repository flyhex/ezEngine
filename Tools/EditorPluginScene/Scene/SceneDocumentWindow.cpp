#include <PCH.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <EditorPluginScene/Scene/SceneDocumentWindow.moc.h>
#include <EditorPluginScene/Scene/SceneDocument.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <CoreUtils/Geometry/GeomUtils.h>
#include <CoreUtils/Geometry/OBJLoader.h>
#include <Foundation/IO/OSFile.h>
#include <QTimer>
#include <QPushButton>
#include <EditorFramework/EngineProcess/EngineProcessMessages.h>
#include <qlayout.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Core/World/GameObject.h>
#include <QKeyEvent>
#include <Foundation/Time/Time.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <ToolsFoundation/Command/TreeCommands.h>

ezSceneDocumentWindow::ezSceneDocumentWindow(ezDocumentBase* pDocument)
  : ezDocumentWindow3D(pDocument)
{
  m_pCenterWidget = new ez3DViewWidget(this, this);

  m_pCenterWidget->setAutoFillBackground(false);
  setCentralWidget(m_pCenterWidget);

  m_bInGizmoInteraction = false;
  SetTargetFramerate(24);

  m_DelegatePropertyEvents = ezMakeDelegate(&ezSceneDocumentWindow::PropertyEventHandler, this);
  m_DelegateDocumentTreeEvents = ezMakeDelegate(&ezSceneDocumentWindow::DocumentTreeEventHandler, this);

  GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(m_DelegateDocumentTreeEvents);
  GetDocument()->GetObjectManager()->m_PropertyEvents.AddEventHandler(m_DelegatePropertyEvents);

  m_Camera.SetCameraMode(ezCamera::CameraMode::PerspectiveFixedFovY, 80.0f, 0.1f, 1000.0f);
  m_Camera.LookAt(ezVec3(0.5f, 1.5f, 2.0f), ezVec3(0.0f, 0.5f, 0.0f), ezVec3(0.0f, 1.0f, 0.0f));

  m_pSelectionContext = EZ_DEFAULT_NEW(ezSelectionContext, pDocument, this, &m_Camera);
  m_pMoveContext = EZ_DEFAULT_NEW(ezCameraMoveContext, m_pCenterWidget, pDocument, this);

  m_pMoveContext->LoadState();
  m_pMoveContext->SetCamera(&m_Camera);

  // add the input contexts in the order in which they are supposed to be processed
  m_pCenterWidget->m_InputContexts.PushBack(m_pSelectionContext);
  m_pCenterWidget->m_InputContexts.PushBack(m_pMoveContext);


  {
    // Menu Bar
    ezMenuBarActionMapView* pMenuBar = static_cast<ezMenuBarActionMapView*>(menuBar());
    ezActionContext context;
    context.m_sMapping = "EditorPluginScene_DocumentMenuBar";
    context.m_pDocument = pDocument;
    pMenuBar->SetActionContext(context);
  }

  {
    // Tool Bar
    ezToolBarActionMapView* pToolBar = new ezToolBarActionMapView(this);
    ezActionContext context;
    context.m_sMapping = "EditorPluginScene_DocumentToolBar";
    context.m_pDocument = pDocument;
    pToolBar->SetActionContext(context);
    pToolBar->setObjectName("SceneDocumentWindow_ToolBar");
    addToolBar(pToolBar);
  }

  ezSceneDocument* pSceneDoc = static_cast<ezSceneDocument*>(GetDocument());
  pSceneDoc->m_SceneEvents.AddEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::DocumentEventHandler, this));

  pSceneDoc->GetSelectionManager()->m_Events.AddEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::SelectionManagerEventHandler, this));

  m_TranslateGizmo.SetDocumentWindow3D(this);
  m_RotateGizmo.SetDocumentWindow3D(this);
  m_ScaleGizmo.SetDocumentWindow3D(this);

  m_TranslateGizmo.SetDocumentGuid(pDocument->GetGuid());
  m_RotateGizmo.SetDocumentGuid(pDocument->GetGuid());
  m_ScaleGizmo.SetDocumentGuid(pDocument->GetGuid());


  m_TranslateGizmo.m_BaseEvents.AddEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::TransformationGizmoEventHandler, this));
  m_RotateGizmo.m_BaseEvents.AddEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::TransformationGizmoEventHandler, this));
  m_ScaleGizmo.m_BaseEvents.AddEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::TransformationGizmoEventHandler, this));
  pSceneDoc->GetObjectManager()->m_PropertyEvents.AddEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::ObjectPropertyEventHandler, this));
}

ezSceneDocumentWindow::~ezSceneDocumentWindow()
{
  ezSceneDocument* pSceneDoc = static_cast<ezSceneDocument*>(GetDocument());
  pSceneDoc->m_SceneEvents.RemoveEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::DocumentEventHandler, this));

  m_TranslateGizmo.m_BaseEvents.RemoveEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::TransformationGizmoEventHandler, this));
  m_RotateGizmo.m_BaseEvents.RemoveEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::TransformationGizmoEventHandler, this));
  m_ScaleGizmo.m_BaseEvents.RemoveEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::TransformationGizmoEventHandler, this));
  pSceneDoc->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::ObjectPropertyEventHandler, this));

  GetDocument()->GetSelectionManager()->m_Events.RemoveEventHandler(ezMakeDelegate(&ezSceneDocumentWindow::SelectionManagerEventHandler, this));

  GetDocument()->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(m_DelegatePropertyEvents);
  GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(m_DelegateDocumentTreeEvents);

  EZ_DEFAULT_DELETE(m_pSelectionContext);
  EZ_DEFAULT_DELETE(m_pMoveContext);
}

void ezSceneDocumentWindow::PropertyEventHandler(const ezDocumentObjectPropertyEvent& e)
{
  m_pEngineView->SendObjectProperties(e);
}

void ezSceneDocumentWindow::UpdateGizmoVisibility()
{
  ezSceneDocument* pSceneDoc = static_cast<ezSceneDocument*>(GetDocument());

  m_TranslateGizmo.SetVisible(false);
  m_RotateGizmo.SetVisible(false);
  m_ScaleGizmo.SetVisible(false);

  if (pSceneDoc->GetSelectionManager()->GetSelection().IsEmpty() || pSceneDoc->GetActiveGizmo() == ActiveGizmo::None)
    return;

  // if world space ...

  switch (pSceneDoc->GetActiveGizmo())
  {
  case ActiveGizmo::Translate:
    m_TranslateGizmo.SetVisible(true);
    break;
  case ActiveGizmo::Rotate:
    m_RotateGizmo.SetVisible(true);
    break;
  case ActiveGizmo::Scale:
    m_ScaleGizmo.SetVisible(true);
    break;
  }
}

void ezSceneDocumentWindow::ObjectPropertyEventHandler(const ezDocumentObjectPropertyEvent& e)
{
  if (m_bInGizmoInteraction)
    return;

  if (!m_TranslateGizmo.IsVisible() && !m_RotateGizmo.IsVisible() && !m_ScaleGizmo.IsVisible())
    return;

  if (e.m_bEditorProperty)
    return;

  if (GetDocument()->GetSelectionManager()->GetSelection().IsEmpty())
    return;

  if (e.m_pObject != GetDocument()->GetSelectionManager()->GetSelection()[0])
    return;

  if (e.m_sPropertyPath == "GlobalPosition")
  {
    UpdateGizmoPosition();
  }
}

void ezSceneDocumentWindow::DocumentEventHandler(const ezSceneDocument::SceneEvent& e)
{
  switch (e.m_Type)
  {
  case ezSceneDocument::SceneEvent::Type::ActiveGizmoChanged:
    {
      UpdateGizmoVisibility();
    }
    break;
  }

}

void ezSceneDocumentWindow::DocumentTreeEventHandler(const ezDocumentObjectStructureEvent& e)
{
  m_pEngineView->SendDocumentTreeChange(e);
}

void ezSceneDocumentWindow::InternalRedraw()
{
  ezDocumentWindow3D::SyncObjects();

  ezEditorInputContext::UpdateActiveInputContext();

  SendRedrawMsg();
}

void ezSceneDocumentWindow::SendRedrawMsg()
{
  ezViewCameraMsgToEngine cam;
  cam.m_fNearPlane = m_Camera.GetNearPlane();
  cam.m_fFarPlane = m_Camera.GetFarPlane();
  cam.m_iCameraMode = (ezInt8)m_Camera.GetCameraMode();
  cam.m_fFovOrDim = m_Camera.GetFovOrDim();
  cam.m_vDirForwards = m_Camera.GetCenterDirForwards();
  cam.m_vDirUp = m_Camera.GetCenterDirUp();
  cam.m_vDirRight = m_Camera.GetCenterDirRight();
  cam.m_vPosition = m_Camera.GetCenterPosition();
  m_Camera.GetViewMatrix(cam.m_ViewMatrix);
  m_Camera.GetProjectionMatrix((float)m_pCenterWidget->width() / (float)m_pCenterWidget->height(), cam.m_ProjMatrix);

  m_pSelectionContext->SetWindowConfig(ezVec2I32(m_pCenterWidget->width(), m_pCenterWidget->height()));

  m_pEngineView->SendMessage(&cam);

  ezViewRedrawMsgToEngine msg;
  msg.m_uiHWND = (ezUInt64)(m_pCenterWidget->winId());
  msg.m_uiWindowWidth = m_pCenterWidget->width();
  msg.m_uiWindowHeight = m_pCenterWidget->height();

  m_pEngineView->SendMessage(&msg);
}

bool ezSceneDocumentWindow::HandleEngineMessage(const ezEditorEngineDocumentMsg* pMsg)
{
  if (ezDocumentWindow3D::HandleEngineMessage(pMsg))
    return true;

  if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezLogMsgToEditor>())
  {
    const ezLogMsgToEditor* pLogMsg = static_cast<const ezLogMsgToEditor*>(pMsg);

    ezLog::Info("Process (%u): '%s'", pLogMsg->m_uiViewID, pLogMsg->m_sText.GetData());

    return true;
  }

  return false;
}

void ezSceneDocumentWindow::UpdateGizmoSelectionList()
{
  // Get the list of all objects that are manipulated
    // and store their original transformation

  m_GizmoSelection.Clear();

  auto hType = ezRTTI::FindTypeByName("ezGameObject");

  auto pSelMan = GetDocument()->GetSelectionManager();
  const auto& Selection = pSelMan->GetSelection();
  for (ezUInt32 sel = 0; sel < Selection.GetCount(); ++sel)
  {
    if (!Selection[sel]->GetTypeAccessor().GetType()->IsDerivedFrom(hType))
      continue;

    // ignore objects, whose parent is already selected as well, so that transformations aren't applied
    // multiple times on the same hierarchy
    if (pSelMan->IsParentSelected(Selection[sel]))
      continue;

    SelectedGO sgo;
    sgo.m_Object = Selection[sel]->GetGuid();
    sgo.m_vTranslation = Selection[sel]->GetTypeAccessor().GetValue("LocalPosition").ConvertTo<ezVec3>();
    sgo.m_vScaling = Selection[sel]->GetTypeAccessor().GetValue("LocalScaling").ConvertTo<ezVec3>();
    sgo.m_Rotation = Selection[sel]->GetTypeAccessor().GetValue("LocalRotation").ConvertTo<ezQuat>();
    sgo.m_ParentTransform = ezSceneDocument::ComputeGlobalTransform(Selection[sel]->GetParent());

    m_GizmoSelection.PushBack(sgo);
  }
}

void ezSceneDocumentWindow::UpdateGizmoPosition()
{
  const auto& LatestSelection = GetDocument()->GetSelectionManager()->GetSelection().PeekBack();

  if (LatestSelection->GetTypeAccessor().GetType() == ezRTTI::FindTypeByName("ezGameObject"))
  {
    const ezTransform tGlobal = ezSceneDocument::ComputeGlobalTransform(LatestSelection);

    ezMat4 mt(tGlobal.m_Rotation, tGlobal.m_vPosition);
    mt.SetScalingFactors(ezVec3(1.0f));

    m_TranslateGizmo.SetTransformation(mt);
    m_RotateGizmo.SetTransformation(mt);
    m_ScaleGizmo.SetTransformation(mt);
  }

}

void ezSceneDocumentWindow::SelectionManagerEventHandler(const ezSelectionManager::Event& e)
{
  switch (e.m_Type)
  {
  case ezSelectionManager::Event::Type::SelectionCleared:
    {
      m_GizmoSelection.IsEmpty();
      UpdateGizmoVisibility();
    }
    break;

  case ezSelectionManager::Event::Type::SelectionSet:
  case ezSelectionManager::Event::Type::ObjectAdded:
    {
      EZ_ASSERT_DEBUG(m_GizmoSelection.IsEmpty(), "This array should have been cleared when the gizmo lost focus");
      UpdateGizmoPosition();

      UpdateGizmoVisibility();
    }
    break;
  }
}

void ezSceneDocumentWindow::TransformationGizmoEventHandler(const ezGizmoBase::BaseEvent& e)
{
  switch (e.m_Type)
  {
  case ezGizmoBase::BaseEvent::Type::BeginInteractions:
    {
      UpdateGizmoSelectionList();

      GetDocument()->GetCommandHistory()->BeginTemporaryCommands();

    }
    break;

  case ezGizmoBase::BaseEvent::Type::EndInteractions:
    {
      GetDocument()->GetCommandHistory()->EndTemporaryCommands(false);

      m_GizmoSelection.Clear();
    }
    break;

  case ezGizmoBase::BaseEvent::Type::Interaction:
    {
      const ezMat4 mTransform = e.m_pGizmo->GetTransformation();

      m_bInGizmoInteraction = true;
      GetDocument()->GetCommandHistory()->StartTransaction();

      bool bCancel = false;

      ezSetObjectPropertyCommand cmd;
      cmd.m_bEditorProperty = false;

      if (e.m_pGizmo == &m_TranslateGizmo)
      {
        cmd.SetPropertyPath("LocalPosition");

        const ezVec3 vTranslate = m_TranslateGizmo.GetTranslationResult();

        for (ezUInt32 sel = 0; sel < m_GizmoSelection.GetCount(); ++sel)
        {
          const auto& obj = m_GizmoSelection[sel];

          ezTransform tGlobal, tLocal;
          tLocal.m_vPosition = obj.m_vTranslation;
          tLocal.m_Rotation = obj.m_Rotation.GetAsMat3();
          //tLocal.m_Rotation.SetScalingFactors(obj.m_vScaling);

          tGlobal.SetGlobalTransform(obj.m_ParentTransform, tLocal);
          tGlobal.m_vPosition += vTranslate;

          tLocal.SetLocalTransform(obj.m_ParentTransform, tGlobal);

          cmd.m_Object = obj.m_Object;
          cmd.m_NewValue = tLocal.m_vPosition;

          if (GetDocument()->GetCommandHistory()->AddCommand(cmd).m_Result.Failed())
          {
            bCancel = true;
            break;
          }
        }
      }

      if (e.m_pGizmo == &m_RotateGizmo)
      {
        cmd.SetPropertyPath("LocalRotation");

        const ezQuat qRotation = m_RotateGizmo.GetRotationResult();

        for (ezUInt32 sel = 0; sel < m_GizmoSelection.GetCount(); ++sel)
        {
          const auto& obj = m_GizmoSelection[sel];

          cmd.m_Object = obj.m_Object;
          cmd.m_NewValue = qRotation * obj.m_Rotation;

          if (GetDocument()->GetCommandHistory()->AddCommand(cmd).m_Result.Failed())
          {
            bCancel = true;
            break;
          }
        }
      }

      if (e.m_pGizmo == &m_ScaleGizmo)
      {
        cmd.SetPropertyPath("LocalScaling");

        const ezVec3 vScale = m_ScaleGizmo.GetScalingResult();

        for (ezUInt32 sel = 0; sel < m_GizmoSelection.GetCount(); ++sel)
        {
          const auto& obj = m_GizmoSelection[sel];

          cmd.m_Object = obj.m_Object;
          cmd.m_NewValue = obj.m_vScaling.CompMult(vScale);

          if (GetDocument()->GetCommandHistory()->AddCommand(cmd).m_Result.Failed())
          {
            bCancel = true;
            break;
          }
        }
      }

      GetDocument()->GetCommandHistory()->EndTransaction(bCancel);
      m_bInGizmoInteraction = false;
    }
    break;
  }

}


