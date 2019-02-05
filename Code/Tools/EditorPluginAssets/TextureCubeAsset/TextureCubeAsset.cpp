#include <PCH.h>

#include <Core/Assets/AssetFileHeader.h>
#include <EditorFramework/Assets/AssetCurator.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <EditorPluginAssets/TextureCubeAsset/TextureCubeAsset.h>
#include <EditorPluginAssets/TextureCubeAsset/TextureCubeAssetManager.h>
#include <EditorPluginAssets/TextureCubeAsset/TextureCubeAssetObjects.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <QStringList>
#include <QTextStream>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/ImageConversion.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTextureCubeAssetDocument, 3, ezRTTINoAllocator);
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezTextureCubeChannelMode, 1)
  EZ_ENUM_CONSTANTS(ezTextureCubeChannelMode::RGB, ezTextureCubeChannelMode::Red, ezTextureCubeChannelMode::Green, ezTextureCubeChannelMode::Blue, ezTextureCubeChannelMode::Alpha)
EZ_END_STATIC_REFLECTED_ENUM;
// clang-format on

const char* ToFilterMode(ezTextureFilterSetting::Enum mode);

ezTextureCubeAssetDocument::ezTextureCubeAssetDocument(const char* szDocumentPath)
  : ezSimpleAssetDocument<ezTextureCubeAssetProperties>(szDocumentPath, true)
{
  m_iTextureLod = -1;
}

ezStatus ezTextureCubeAssetDocument::RunTexConv(const char* szTargetFile, const ezAssetFileHeader& AssetHeader, bool bUpdateThumbnail)
{
  const bool bRunTexConv2 = false;

  const ezTextureCubeAssetProperties* pProp = GetProperties();

  QStringList arguments;
  ezStringBuilder temp;

  // Asset Version
  {
    arguments << "-assetVersion";
    arguments << ezConversionUtils::ToString(AssetHeader.GetFileVersion(), temp).GetData();
  }

  // Asset Hash
  {
    const ezUInt64 uiHash64 = AssetHeader.GetFileHash();
    const ezUInt32 uiHashLow32 = uiHash64 & 0xFFFFFFFF;
    const ezUInt32 uiHashHigh32 = (uiHash64 >> 32) & 0xFFFFFFFF;

    temp.Format("{0}", ezArgU(uiHashLow32, 8, true, 16, true));
    arguments << "-assetHashLow";
    arguments << temp.GetData();

    temp.Format("{0}", ezArgU(uiHashHigh32, 8, true, 16, true));
    arguments << "-assetHashHigh";
    arguments << temp.GetData();
  }


  arguments << "-out";
  arguments << szTargetFile;

  const ezStringBuilder sThumbnail = GetThumbnailFilePath();
  if (bUpdateThumbnail)
  {
    // Thumbnail
    const ezStringBuilder sDir = sThumbnail.GetFileDirectory();
    ezOSFile::CreateDirectoryStructure(sDir);

    if (bRunTexConv2)
    {
      arguments << "-thumbnailRes";
      arguments << "256";
      arguments << "-thumbnailOut";
    }
    else
    {
      arguments << "-thumbnail";
    }

    arguments << QString::fromUtf8(sThumbnail.GetData());
  }

  // TODO: downscale steps and min/max resolution
  // TODO: hdr exposure

  if (bRunTexConv2)
  {
    arguments << "-mipmaps";

    // TODO: more mipmap modes ?
    if (pProp->m_bMipmaps)
    {
      arguments << "Linear";
    }
    else
    {
      arguments << "None";
    }

    arguments << "-compression";

    // TODO: more compression modes
    if (pProp->m_bCompression)
    {
      arguments << "Medium";
    }
    else
    {
      arguments << "None";
    }

    // TODO: better usage mode
    arguments << "-usage";

    if (pProp->IsSRGB())
    {
      arguments << "Color";
    }
    else if (pProp->IsHDR())
    {
      arguments << "Hdr";
    }
    else
    {
      arguments << "Linear";
    }

    arguments << "-filter" << ToFilterMode(pProp->m_TextureFilter);

    arguments << "-type";
    arguments << "Cubemap";
  }
  else
  {
    arguments << "-channels";
    arguments << ezConversionUtils::ToString(pProp->GetNumChannels(), temp).GetData();

    if (pProp->m_bMipmaps)
      arguments << "-mipmaps";

    if (pProp->m_bCompression)
      arguments << "-compress";

    if (pProp->IsSRGB())
      arguments << "-srgb";

    if (pProp->IsHDR())
      arguments << "-hdr";

    arguments << "-filter" << QString::number(pProp->m_TextureFilter.GetValue());

    arguments << "-cubemap";
  }

  if (pProp->m_bPremultipliedAlpha)
    arguments << "-premulalpha";

  const ezInt32 iNumInputFiles = pProp->GetNumInputFiles();
  for (ezInt32 i = 0; i < iNumInputFiles; ++i)
  {
    temp.Format("-in{0}", i);

    if (ezStringUtils::IsNullOrEmpty(pProp->GetInputFile(i)))
      break;

    arguments << temp.GetData();
    arguments << QString(pProp->GetAbsoluteInputFilePath(i).GetData());
  }

  ezStringBuilder cmd;
  for (ezInt32 i = 0; i < arguments.size(); ++i)
    cmd.Append(" ", arguments[i].toUtf8().data());

  if (bRunTexConv2)
  {
    ezLog::Debug("TexConv.exe2{0}", cmd);

    EZ_SUCCEED_OR_RETURN(ezQtEditorApp::GetSingleton()->ExecuteTool("TexConv2.exe", arguments, 60, ezLog::GetThreadLocalLogSystem()));
  }
  else
  {
    ezLog::Debug("TexConv.exe{0}", cmd);

    EZ_SUCCEED_OR_RETURN(ezQtEditorApp::GetSingleton()->ExecuteTool("TexConv.exe", arguments, 60, ezLog::GetThreadLocalLogSystem()));
  }

  if (bUpdateThumbnail)
  {
    ezUInt64 uiThumbnailHash = ezAssetCurator::GetSingleton()->GetAssetReferenceHash(GetGuid());
    EZ_ASSERT_DEV(uiThumbnailHash != 0, "Thumbnail hash should never be zero when reaching this point!");
    ezAssetFileHeader assetThumbnailHeader;
    assetThumbnailHeader.SetFileHashAndVersion(uiThumbnailHash, GetAssetTypeVersion());
    AppendThumbnailInfo(sThumbnail, assetThumbnailHeader);
    InvalidateAssetThumbnail();
  }

  return ezStatus(EZ_SUCCESS);
}

ezStatus ezTextureCubeAssetDocument::InternalTransformAsset(const char* szTargetFile, const char* szOutputTag,
  const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, bool bTriggeredManually)
{
  // EZ_ASSERT_DEV(ezStringUtils::IsEqual(szPlatform, "PC"), "Platform '{0}' is not supported", szPlatform);
  const bool bUpdateThumbnail = pAssetProfile == ezAssetCurator::GetSingleton()->GetDevelopmentAssetProfile();

  ezStatus result = RunTexConv(szTargetFile, AssetHeader, bUpdateThumbnail);

  ezFileStats stat;
  if (ezOSFile::GetFileStats(szTargetFile, stat).Succeeded() && stat.m_uiFileSize == 0)
  {
    // if the file was touched, but nothing written to it, delete the file
    // might happen if TexConv crashed or had an error
    ezOSFile::DeleteFile(szTargetFile);
    result.m_Result = EZ_FAILURE;
  }

  return result;
}

const char* ezTextureCubeAssetDocument::QueryAssetType() const
{
  return "Texture Cube";
}

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTextureCubeAssetDocumentGenerator, 1, ezRTTIDefaultAllocator<ezTextureCubeAssetDocumentGenerator>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezTextureCubeAssetDocumentGenerator::ezTextureCubeAssetDocumentGenerator()
{
  AddSupportedFileType("dds");
  AddSupportedFileType("hdr");

  // these formats would need to use 6 files for the faces
  // more elaborate detection and mapping would need to be implemented
  // AddSupportedFileType("tga");
  // AddSupportedFileType("jpg");
  // AddSupportedFileType("jpeg");
  // AddSupportedFileType("png");
}

ezTextureCubeAssetDocumentGenerator::~ezTextureCubeAssetDocumentGenerator() {}

void ezTextureCubeAssetDocumentGenerator::GetImportModes(
  const char* szParentDirRelativePath, ezHybridArray<ezAssetDocumentGenerator::Info, 4>& out_Modes) const
{
  ezStringBuilder baseOutputFile = szParentDirRelativePath;

  const ezStringBuilder baseFilename = baseOutputFile.GetFileName();
  const bool isHDR = ezPathUtils::HasExtension(szParentDirRelativePath, "hdr");

  /// \todo Make this configurable
  const bool isCubemap =
    ((baseFilename.FindSubString_NoCase("cubemap") != nullptr) || (baseFilename.FindSubString_NoCase("skybox") != nullptr));

  baseOutputFile.ChangeFileExtension(GetDocumentExtension());

  if (isHDR)
  {
    {
      ezAssetDocumentGenerator::Info& info = out_Modes.ExpandAndGetRef();
      info.m_Priority = isCubemap ? ezAssetDocGeneratorPriority::HighPriority : ezAssetDocGeneratorPriority::Undecided;
      info.m_sName = "CubemapImport.SkyboxHDR";
      info.m_sOutputFileParentRelative = baseOutputFile;
      info.m_sIcon = ":/AssetIcons/Texture_Cube.png";
    }
  }
  else
  {
    {
      ezAssetDocumentGenerator::Info& info = out_Modes.ExpandAndGetRef();
      info.m_Priority = isCubemap ? ezAssetDocGeneratorPriority::HighPriority : ezAssetDocGeneratorPriority::Undecided;
      info.m_sName = "CubemapImport.Skybox";
      info.m_sOutputFileParentRelative = baseOutputFile;
      info.m_sIcon = ":/AssetIcons/Texture_Cube.png";
    }
  }
}

ezStatus ezTextureCubeAssetDocumentGenerator::Generate(
  const char* szDataDirRelativePath, const ezAssetDocumentGenerator::Info& info, ezDocument*& out_pGeneratedDocument)
{
  auto pApp = ezQtEditorApp::GetSingleton();

  out_pGeneratedDocument = pApp->CreateDocument(info.m_sOutputFileAbsolute, ezDocumentFlags::None);
  if (out_pGeneratedDocument == nullptr)
    return ezStatus("Could not create target document");

  ezTextureCubeAssetDocument* pAssetDoc = ezDynamicCast<ezTextureCubeAssetDocument*>(out_pGeneratedDocument);
  if (pAssetDoc == nullptr)
    return ezStatus("Target document is not a valid ezTextureCubeAssetDocument");

  auto& accessor = pAssetDoc->GetPropertyObject()->GetTypeAccessor();
  accessor.SetValue("Input1", szDataDirRelativePath);
  accessor.SetValue("ChannelMapping", (int)ezTextureCubeChannelMappingEnum::RGB1);

  if (info.m_sName == "CubemapImport.SkyboxHDR")
  {
    accessor.SetValue("Usage", (int)ezTextureCubeUsageEnum::SkyboxHDR);
  }
  else if (info.m_sName == "CubemapImport.Skybox")
  {
    accessor.SetValue("Usage", (int)ezTextureCubeUsageEnum::Skybox);
  }

  return ezStatus(EZ_SUCCESS);
}
