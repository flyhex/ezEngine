#include <GuiFoundation/PCH.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <ToolsFoundation/Project/ToolsProject.h>
#include <Foundation/IO/OSFile.h>
#include <QProcess>
#include <QDir>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezDocumentAction, ezButtonAction, 0, ezRTTINoAllocator);
EZ_END_DYNAMIC_REFLECTED_TYPE();

////////////////////////////////////////////////////////////////////////
// ezDocumentActions
////////////////////////////////////////////////////////////////////////

ezActionDescriptorHandle ezDocumentActions::s_hSave;
ezActionDescriptorHandle ezDocumentActions::s_hSaveAs;
ezActionDescriptorHandle ezDocumentActions::s_hSaveAll;
ezActionDescriptorHandle ezDocumentActions::s_hClose;
ezActionDescriptorHandle ezDocumentActions::s_hOpenContainingFolder;

void ezDocumentActions::RegisterActions()
{
  ezHashedString sCategory;
  sCategory.Assign("Document");

  s_hSave = ezActionManager::RegisterAction(ezActionDescriptor(ezActionType::Action, ezActionScope::Document, "Save", sCategory, &CreateSaveAction));
  s_hSaveAs = ezActionManager::RegisterAction(ezActionDescriptor(ezActionType::Action, ezActionScope::Document, "Save As", sCategory, &CreateSaveAsAction));
  s_hSaveAll = ezActionManager::RegisterAction(ezActionDescriptor(ezActionType::Action, ezActionScope::Document, "Save All", sCategory, &CreateSaveAllAction));
  s_hClose = ezActionManager::RegisterAction(ezActionDescriptor(ezActionType::Action, ezActionScope::Document, "Close", sCategory, &CreateCloseAction));
  s_hOpenContainingFolder = ezActionManager::RegisterAction(ezActionDescriptor(ezActionType::Action, ezActionScope::Document, "Open Containing Folder", sCategory, &CreateOpenContainingFolderAction));

}

void ezDocumentActions::UnregisterActions()
{
  ezActionManager::UnregisterAction(s_hSave);
  ezActionManager::UnregisterAction(s_hSaveAs);
  ezActionManager::UnregisterAction(s_hSaveAll);
  ezActionManager::UnregisterAction(s_hClose);
  ezActionManager::UnregisterAction(s_hOpenContainingFolder);

  s_hSave.Invalidate();
  s_hSaveAs.Invalidate();
  s_hSaveAll.Invalidate();
  s_hClose.Invalidate();
  s_hOpenContainingFolder.Invalidate();
}

void ezDocumentActions::MapActions(const char* szMapping, const ezHashedString& sPath)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(szMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "The given mapping ('%s') does not exist, mapping the documents actions failed!", szMapping);

  ezActionMapDescriptor desc;
  desc.m_sPath = sPath;

  desc.m_hAction = s_hSave;
  desc.m_fOrder = 1.0f;
  EZ_VERIFY(pMap->MapAction(desc).IsValid(), "Mapping failed");

  desc.m_hAction = s_hSaveAs;
  desc.m_fOrder = 2.0f;
  EZ_VERIFY(pMap->MapAction(desc).IsValid(), "Mapping failed");

  desc.m_hAction = s_hSaveAll;
  desc.m_fOrder = 3.0f;
  EZ_VERIFY(pMap->MapAction(desc).IsValid(), "Mapping failed");

  desc.m_hAction = s_hClose;
  desc.m_fOrder = 4.0f;
  EZ_VERIFY(pMap->MapAction(desc).IsValid(), "Mapping failed");

  desc.m_hAction = s_hOpenContainingFolder;
  desc.m_fOrder = 5.0f;
  EZ_VERIFY(pMap->MapAction(desc).IsValid(), "Mapping failed");
}

ezAction* ezDocumentActions::CreateSaveAction(const ezActionContext& context)
{
  return EZ_DEFAULT_NEW(ezDocumentAction)(context, "Save", ezDocumentAction::DocumentButton::Save);
}

ezAction* ezDocumentActions::CreateSaveAsAction(const ezActionContext& context)
{
  return EZ_DEFAULT_NEW(ezDocumentAction)(context, "Save As...", ezDocumentAction::DocumentButton::SaveAs);
}

ezAction* ezDocumentActions::CreateSaveAllAction(const ezActionContext& context)
{
  return EZ_DEFAULT_NEW(ezDocumentAction)(context, "Save All", ezDocumentAction::DocumentButton::SaveAll);
}

ezAction* ezDocumentActions::CreateCloseAction(const ezActionContext& context)
{
  return EZ_DEFAULT_NEW(ezDocumentAction)(context, "Close", ezDocumentAction::DocumentButton::Close);
}

ezAction* ezDocumentActions::CreateOpenContainingFolderAction(const ezActionContext& context)
{
  return EZ_DEFAULT_NEW(ezDocumentAction)(context, "Open Containing Folder", ezDocumentAction::DocumentButton::OpenContainingFolder);
}

////////////////////////////////////////////////////////////////////////
// ezDocumentAction
////////////////////////////////////////////////////////////////////////

ezDocumentAction::ezDocumentAction(const ezActionContext& context, const char* szName, DocumentButton button)
  : ezButtonAction(context, szName, false)
{
  m_ButtonType = button;

  if (context.m_pDocument == nullptr)
  {
    if (button == DocumentButton::Save || button == DocumentButton::SaveAs)
    {
      // for actions that require a document, hide them
      SetVisible(false);
    }
  }
  else
  {
    m_Context.m_pDocument->m_EventsOne.AddEventHandler(ezMakeDelegate(&ezDocumentAction::DocumentEventHandler, this));

    if (m_ButtonType == DocumentButton::Save)
    {
      SetVisible(!m_Context.m_pDocument->IsReadOnly());
      SetEnabled(m_Context.m_pDocument->IsModified());
    }
  }
}

ezDocumentAction::~ezDocumentAction()
{
  if (m_Context.m_pDocument)
  {
    m_Context.m_pDocument->m_EventsOne.RemoveEventHandler(ezMakeDelegate(&ezDocumentAction::DocumentEventHandler, this));
  }
}

void ezDocumentAction::DocumentEventHandler(const ezDocumentBase::Event& e)
{
  switch (e.m_Type)
  {
  case ezDocumentBase::Event::Type::DocumentSaved:
  case ezDocumentBase::Event::Type::ModifiedChanged:
    {
      if (m_ButtonType == DocumentButton::Save)
      {
        SetEnabled(m_Context.m_pDocument->IsModified());
        TriggerUpdate();
      }
    }
    break;
  }
}

void ezDocumentAction::Execute(const ezVariant& value)
{
  switch (m_ButtonType)
  {
  case ezDocumentAction::DocumentButton::Save:
    {
      if (m_Context.m_pDocument->SaveDocument().m_Result.Failed())
      {
        /// \todo Error box
      }
    }
    break;

  case ezDocumentAction::DocumentButton::SaveAs:
    /// \todo Save as
    break;

  case ezDocumentAction::DocumentButton::SaveAll:
    {
      for (auto pMan : ezDocumentManagerBase::GetAllDocumentManagers())
      {
        for (auto pDoc : pMan->ezDocumentManagerBase::GetAllDocuments())
        {
          if (pDoc->SaveDocument().m_Result.Failed())
          {
            /// \todo Error box (or rather use the document window)
          }
        }
      }
    }
    break;

  case ezDocumentAction::DocumentButton::Close:
    {
      /// \todo Use the window
      if (m_Context.m_pDocument)
      {
        m_Context.m_pDocument->GetDocumentManager()->CloseDocument(m_Context.m_pDocument);
      }
    }
    break;

  case ezDocumentAction::DocumentButton::OpenContainingFolder:
    {
      ezString sPath;

      if (!m_Context.m_pDocument)
      {
        if (ezToolsProject::IsProjectOpen())
          sPath = ezToolsProject::GetInstance()->GetProjectPath();
        else
          sPath = ezOSFile::GetApplicationDirectory();
      }
      else
        sPath = m_Context.m_pDocument->GetDocumentPath();

      QStringList args;
      args << "/select," << QDir::toNativeSeparators(sPath.GetData());
      QProcess::startDetached("explorer", args);

    }
    break;
  }
}

