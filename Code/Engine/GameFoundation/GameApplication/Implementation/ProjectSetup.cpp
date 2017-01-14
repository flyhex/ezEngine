#include <GameFoundation/PCH.h>
#include <GameFoundation/GameApplication/GameApplication.h>
#include <Core/Application/Config/PluginConfig.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
#include <RendererDX11/Device/DeviceDX11.h>
typedef ezGALDeviceDX11 ezGALDeviceDefault;
#else
/// \todo We might need a dummy graphics device type
//#include <RendererGL/Device/DeviceGL.h>
//typedef ezGALDeviceGL ezGALDeviceDefault;
#endif
#include <Foundation/IO/FileSystem/FileReader.h>
#include <GameUtils/Prefabs/PrefabResource.h>
#include <GameUtils/Collection/CollectionResource.h>
#include <RendererCore/Material/MaterialResource.h>
#include <GameUtils/Surfaces/SurfaceResource.h>
#include <GameUtils/Curves/ColorGradientResource.h>
#include <GameUtils/Curves/Curve1DResource.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <RendererCore/Textures/TextureCubeResource.h>

void ezGameApplication::DoProjectSetup()
{
  DoSetupLogWriters();

  ezTelemetry::CreateServer();

  ezApplicationConfig::SetProjectDirectory(FindProjectDirectory());

  DoConfigureFileSystem();
  DoConfigureAssetManagement();
  DoSetupDataDirectories();
  DoLoadCustomPlugins();
  DoLoadPluginsFromConfig();
  DoSetupGraphicsDevice();
  DoSetupDefaultResources();
  DoConfigureInput(false);
  DoLoadTags();

  ezTaskSystem::SetTargetFrameTime(1000.0 / 20.0);
}

void ezGameApplication::DoSetupLogWriters()
{
  ezGlobalLog::AddLogWriter(ezLogWriter::Console::LogMessageHandler);
  ezGlobalLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);
}

void ezGameApplication::DoConfigureFileSystem()
{
  ezFileSystem::RegisterDataDirectoryFactory(ezDataDirectory::FolderType::Factory);
}


void ezGameApplication::DoSetupDataDirectories()
{
  const ezString sUserData = ezOSFile::GetUserDataFolder(m_sAppName);

  ezOSFile::CreateDirectoryStructure(sUserData);

  ezFileSystem::AddDataDirectory("", "GameApplication", ":", ezFileSystem::ReadOnly); // for absolute paths
  ezFileSystem::AddDataDirectory(ezOSFile::GetApplicationDirectory(), "GameApplication", "bin", ezFileSystem::AllowWrites); // writing to the binary directory
  ezFileSystem::AddDataDirectory(ezOSFile::GetApplicationDirectory(), "GameApplication", "shadercache", ezFileSystem::AllowWrites); // for shader files
  ezFileSystem::AddDataDirectory(sUserData, "GameApplication", "appdata", ezFileSystem::AllowWrites); // for writing app user data

  ezApplicationFileSystemConfig appFileSystemConfig;
  appFileSystemConfig.Load();
  appFileSystemConfig.Apply();
}

void ezGameApplication::DoConfigureAssetManagement()
{
  // which redirection table to search
  ezDataDirectory::FolderType::s_sRedirectionFile = "AssetCache/PC.ezAidlt";

  // which platform assets to use
  ezDataDirectory::FolderType::s_sRedirectionPrefix = "AssetCache/";

  ezResourceManager::RegisterResourceForAssetType("Collection", ezGetStaticRTTI<ezCollectionResource>());
  // collision mesh resources registered by PhysX plugin
  ezResourceManager::RegisterResourceForAssetType("Material", ezGetStaticRTTI<ezMaterialResource>());
  ezResourceManager::RegisterResourceForAssetType("Mesh", ezGetStaticRTTI<ezMeshResource>());
  ezResourceManager::RegisterResourceForAssetType("Prefab", ezGetStaticRTTI<ezPrefabResource>());
  ezResourceManager::RegisterResourceForAssetType("RenderPipeline", ezGetStaticRTTI<ezRenderPipelineResource>());
  // sound bank resource registered by Fmod plugin
  ezResourceManager::RegisterResourceForAssetType("Surface", ezGetStaticRTTI<ezSurfaceResource>());
  ezResourceManager::RegisterResourceForAssetType("Texture 2D", ezGetStaticRTTI<ezTexture2DResource>());
  ezResourceManager::RegisterResourceForAssetType("Texture 3D", ezGetStaticRTTI<ezTexture2DResource>());
  ezResourceManager::RegisterResourceForAssetType("Texture Cube", ezGetStaticRTTI<ezTexture2DResource>());
}

void ezGameApplication::DoSetupDefaultResources()
{
  // Shaders
  {
    ezShaderResourceDescriptor desc;
    ezShaderResourceHandle hMissingShader = ezResourceManager::CreateResource<ezShaderResource>("MissingShaderResource", desc, "MissingShaderResource");

    ezShaderResource::SetTypeMissingResource(hMissingShader);
  }

  // 2D Textures
  {
    ezTexture2DResourceHandle hFallbackTexture = ezResourceManager::LoadResource<ezTexture2DResource>("Textures/LoadingTexture_D.dds");
    ezTexture2DResourceHandle hMissingTexture = ezResourceManager::LoadResource<ezTexture2DResource>("Textures/MissingTexture_D.dds");

    ezTexture2DResource::SetTypeFallbackResource(hFallbackTexture);
    ezTexture2DResource::SetTypeMissingResource(hMissingTexture);
  }

  // Cube Textures
  {
    /// \todo Loading Cubemap Texture

    ezTextureCubeResourceHandle hFallbackTexture = ezResourceManager::LoadResource<ezTextureCubeResource>("Textures/MissingCubeMap.dds");
    ezTextureCubeResourceHandle hMissingTexture = ezResourceManager::LoadResource<ezTextureCubeResource>("Textures/MissingCubeMap.dds");

    ezTextureCubeResource::SetTypeFallbackResource(hFallbackTexture);
    ezTextureCubeResource::SetTypeMissingResource(hMissingTexture);
  }

  // Materials
  {
    ezMaterialResourceHandle hMissingMaterial = ezResourceManager::LoadResource<ezMaterialResource>("Materials/BaseMaterials/MissingMaterial.ezMaterial");
    ezMaterialResourceHandle hFallbackMaterial = ezResourceManager::LoadResource<ezMaterialResource>("Materials/BaseMaterials/LoadingMaterial.ezMaterial");

    ezMaterialResource::SetTypeFallbackResource(hFallbackMaterial);
    ezMaterialResource::SetTypeMissingResource(hMissingMaterial);
  }

  // Meshes
  {
    ezMeshResourceHandle hMissingMesh = ezResourceManager::LoadResource<ezMeshResource>("Meshes/MissingMesh.ezMesh");
    ezMeshResource::SetTypeMissingResource(hMissingMesh);
  }

  // Prefabs
  {
    //ezPrefabResourceDescriptor emptyPrefab;
    //ezPrefabResourceHandle hMissingPrefab = ezResourceManager::CreateResource<ezPrefabResource>("MissingPrefabResource", emptyPrefab, "MissingPrefabResource");

    ezPrefabResourceHandle hMissingPrefab = ezResourceManager::LoadResource<ezPrefabResource>("Prefabs/MissingPrefab.ezObjectGraph");
    ezPrefabResource::SetTypeMissingResource(hMissingPrefab);
  }

  // Collections
  {
    ezCollectionResourceDescriptor desc;
    ezCollectionResourceHandle hMissingCollection = ezResourceManager::CreateResource<ezCollectionResource>("MissingCollectionResource", desc, "MissingCollectionResource");

    ezCollectionResource::SetTypeMissingResource(hMissingCollection);
  }

  // Render Pipelines
  {
    ezRenderPipelineResourceHandle hMissingRenderPipeline = ezRenderPipelineResource::CreateMissingPipeline();
    ezRenderPipelineResource::SetTypeMissingResource(hMissingRenderPipeline);
  }

  // Color Gradient
  {
    ezColorGradientResourceDescriptor cg;
    cg.m_Gradient.AddColorControlPoint(0, ezColor::RebeccaPurple);
    cg.m_Gradient.AddColorControlPoint(1, ezColor::LawnGreen);

    ezColorGradientResourceHandle hResource = ezResourceManager::CreateResource<ezColorGradientResource>("MissingColorGradient", cg, "Missing Color Gradient Resource");
    ezColorGradientResource::SetTypeMissingResource(hResource);
  }

  // 1D Curve
  {
    ezCurve1DResourceDescriptor cd;
    auto& curve = cd.m_Curves.ExpandAndGetRef();
    curve.AddControlPoint(0);
    curve.AddControlPoint(1);

    ezCurve1DResourceHandle hResource = ezResourceManager::CreateResource<ezCurve1DResource>("MissingCurve1D", cd, "Missing Curve1D Resource");
    ezCurve1DResource::SetTypeMissingResource(hResource);
  }
}


void ezGameApplication::DoSetupGraphicsDevice()
{
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  ezGALDeviceCreationDescription DeviceInit;
  DeviceInit.m_bCreatePrimarySwapChain = false;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
  DeviceInit.m_bDebugDevice = true;
#endif

  ezGALDevice* pDevice = EZ_DEFAULT_NEW(ezGALDeviceDefault, DeviceInit);
  EZ_VERIFY(pDevice->Init() == EZ_SUCCESS, "Graphics device creation failed!");

  ezGALDevice::SetDefaultDevice(pDevice);

  // Create GPU resource pool
  ezGPUResourcePool* pResourcePool = EZ_DEFAULT_NEW(ezGPUResourcePool);
  ezGPUResourcePool::SetDefaultInstance(pResourcePool);

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  ezShaderManager::Configure("DX11_SM50", true);
#else
  ezShaderManager::Configure("GL3", true);
#endif

#endif
}

void ezGameApplication::DoLoadCustomPlugins()
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  EZ_VERIFY(ezPlugin::LoadPlugin("ezInspectorPlugin").Succeeded(), "Could not load Inspector Plugin.");

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  EZ_VERIFY(ezPlugin::LoadPlugin("ezShaderCompilerHLSL").Succeeded(), "Could not load HLSL Shader Compiler Plugin.");
#endif

#endif
}


void ezGameApplication::DoLoadPluginsFromConfig()
{
  ezApplicationPluginConfig appPluginConfig;
  appPluginConfig.Load();
  appPluginConfig.SetOnlyLoadManualPlugins(true);
  appPluginConfig.Apply();
}

void ezGameApplication::DoLoadTags()
{
  EZ_LOG_BLOCK("Reading Tags", "Tags.ddl");

  ezFileReader file;
  if (file.Open("Tags.ddl").Failed())
  {
    ezLog::Dev("'Tags.ddl' does not exist");
    return;
  }

  ezStringBuilder tmp;

  ezOpenDdlReader reader;
  if (reader.ParseDocument(file).Failed())
  {
    ezLog::Error("Failed to parse DDL data in tags file");
    return;
  }

  const ezOpenDdlReaderElement* pRoot = reader.GetRootElement();

  for (const ezOpenDdlReaderElement* pTags = pRoot->GetFirstChild(); pTags != nullptr; pTags = pTags->GetSibling())
  {
    if (!pTags->IsCustomType("Tag"))
      continue;

    const ezOpenDdlReaderElement* pName = pTags->FindChildOfType(ezOpenDdlPrimitiveType::String, "Name");

    if (!pName)
    {
      ezLog::Error("Incomplete tag declaration!");
      continue;
    }

    tmp = pName->GetPrimitivesString()[0];
    ezTagRegistry::GetGlobalRegistry().RegisterTag(tmp);
  }
}

void ezGameApplication::DoShutdownGraphicsDevice()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  for (ezUInt32 i = 0; i < m_Windows.GetCount(); ++i)
  {
    if(m_Windows[i].m_hSwapChain != pDevice->GetPrimarySwapChain())
      pDevice->DestroySwapChain(m_Windows[i].m_hSwapChain);
  }

  // Cleanup resource pool
  ezGPUResourcePool::SetDefaultInstance(nullptr);

  ezResourceManager::FreeUnusedResources(true);

  pDevice->Shutdown();
  EZ_DEFAULT_DELETE(pDevice);
  ezGALDevice::SetDefaultDevice(nullptr);
}

void ezGameApplication::DoShutdownLogWriters()
{

}

