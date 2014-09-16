#include "Log.hpp"
#include "Manager.hpp"
#include "HookManager.hpp"

#include <d3d9.h>
#include <d3dx9shader.h>
#include <initguid.h>

static LPCSTR													DXGetErrorStringA(HRESULT hr)
{
	switch (hr)
	{
		default:
			__declspec(thread) static CHAR buf[20];
			sprintf_s(buf, "0x%lx", hr);
			return buf;
		case D3DERR_NOTAVAILABLE:
			return "D3DERR_NOTAVAILABLE";
		case D3DERR_INVALIDCALL:
			return "D3DERR_INVALIDCALL";
		case D3DERR_INVALIDDEVICE:
			return "D3DERR_INVALIDDEVICE";
		case D3DERR_DEVICEHUNG:
			return "D3DERR_DEVICEHUNG";
		case D3DERR_DEVICELOST:
			return "D3DERR_DEVICELOST";
		case D3DERR_DEVICENOTRESET:
			return "D3DERR_DEVICENOTRESET";
		case D3DERR_WASSTILLDRAWING:
			return "D3DERR_WASSTILLDRAWING";
	}
}

// -----------------------------------------------------------------------------------------------------

struct															D3DADAPTER_IDENTIFIER8
{
	char														Driver[MAX_DEVICE_IDENTIFIER_STRING];
	char														Description[MAX_DEVICE_IDENTIFIER_STRING];
	LARGE_INTEGER												DriverVersion;
	DWORD														VendorId;
	DWORD														DeviceId;
	DWORD														SubSysId;
	DWORD														Revision;
	GUID														DeviceIdentifier;
	DWORD														WHQLLevel;
};
struct															D3DCAPS8
{
	D3DDEVTYPE													DeviceType;
	UINT														AdapterOrdinal;
	DWORD														Caps;
	DWORD														Caps2;
	DWORD														Caps3;
	DWORD														PresentationIntervals;
	DWORD														CursorCaps;
	DWORD														DevCaps;
	DWORD														PrimitiveMiscCaps;
	DWORD														RasterCaps;
	DWORD														ZCmpCaps;
	DWORD														SrcBlendCaps;
	DWORD														DestBlendCaps;
	DWORD														AlphaCmpCaps;
	DWORD														ShadeCaps;
	DWORD														TextureCaps;
	DWORD														TextureFilterCaps;
	DWORD														CubeTextureFilterCaps;
	DWORD														VolumeTextureFilterCaps;
	DWORD														TextureAddressCaps;
	DWORD														VolumeTextureAddressCaps;
	DWORD														LineCaps;
	DWORD														MaxTextureWidth, MaxTextureHeight;
	DWORD														MaxVolumeExtent;
	DWORD														MaxTextureRepeat;
	DWORD														MaxTextureAspectRatio;
	DWORD														MaxAnisotropy;
	float														MaxVertexW;
	float														GuardBandLeft, GuardBandTop, GuardBandRight, GuardBandBottom;
	float														ExtentsAdjust;
	DWORD														StencilCaps;
	DWORD														FVFCaps;
	DWORD														TextureOpCaps;
	DWORD														MaxTextureBlendStages;
	DWORD														MaxSimultaneousTextures;
	DWORD														VertexProcessingCaps;
	DWORD														MaxActiveLights;
	DWORD														MaxUserClipPlanes;
	DWORD														MaxVertexBlendMatrices;
	DWORD														MaxVertexBlendMatrixIndex;
	float														MaxPointSize;
	DWORD														MaxPrimitiveCount;
	DWORD														MaxVertexIndex;
	DWORD														MaxStreams;
	DWORD														MaxStreamStride;
	DWORD														VertexShaderVersion;
	DWORD														MaxVertexShaderConst;
	DWORD														PixelShaderVersion;
	float														MaxPixelShaderValue;
};
struct															D3DPRESENT_PARAMETERS8
{
	UINT														BackBufferWidth;
	UINT														BackBufferHeight;
	D3DFORMAT													BackBufferFormat;
	UINT														BackBufferCount;
	D3DMULTISAMPLE_TYPE											MultiSampleType;
	D3DSWAPEFFECT												SwapEffect;
	HWND														hDeviceWindow;
	BOOL														Windowed;
	BOOL														EnableAutoDepthStencil;
	D3DFORMAT													AutoDepthStencilFormat;
	DWORD														Flags;
	UINT														FullScreen_RefreshRateInHz;
	UINT														FullScreen_PresentationInterval;
};
typedef D3DCLIPSTATUS9											D3DCLIPSTATUS8;
typedef D3DVIEWPORT9											D3DVIEWPORT8;
typedef D3DMATERIAL9											D3DMATERIAL8;
typedef D3DLIGHT9												D3DLIGHT8;

struct															IDirect3D8;
struct															IDirect3DDevice8;
struct															IDirect3DResource8;
struct															IDirect3DBaseTexture8;
struct															IDirect3DTexture8;
struct															IDirect3DVolumeTexture8;
struct															IDirect3DCubeTexture8;
struct															IDirect3DVertexBuffer8;
struct															IDirect3DIndexBuffer8;
struct															IDirect3DSurface8;
struct															IDirect3DVolume8;
struct															IDirect3DSwapChain8;

struct															IDirect3D8 : public IUnknown
{
	IDirect3D8(HMODULE hModule, IDirect3D9 *pProxyD3D) : mModule(hModule), mProxy(pProxyD3D)
	{
	}
	~IDirect3D8(void)
	{
		::FreeLibrary(this->mModule);
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE								AddRef(void);
	virtual ULONG STDMETHODCALLTYPE								Release(void);

	virtual HRESULT STDMETHODCALLTYPE							RegisterSoftwareDevice(void *pInitializeFunction);
	virtual UINT STDMETHODCALLTYPE								GetAdapterCount(void);
	virtual HRESULT STDMETHODCALLTYPE							GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier);
	virtual UINT STDMETHODCALLTYPE								GetAdapterModeCount(UINT Adapter);
	virtual HRESULT STDMETHODCALLTYPE							EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode);
	virtual HRESULT STDMETHODCALLTYPE							GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode);
	virtual HRESULT STDMETHODCALLTYPE							CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
	virtual HRESULT STDMETHODCALLTYPE							CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
	virtual HRESULT STDMETHODCALLTYPE							CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType);
	virtual HRESULT STDMETHODCALLTYPE							CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
	virtual HRESULT STDMETHODCALLTYPE							GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps);
	virtual HMONITOR STDMETHODCALLTYPE							GetAdapterMonitor(UINT Adapter);
	virtual HRESULT STDMETHODCALLTYPE							CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS8 *pPresentationParameters, IDirect3DDevice8 **ppReturnedDeviceInterface);

	HMODULE														mModule;
	IDirect3D9 *												mProxy;
};
struct															IDirect3DDevice8 : public IUnknown
{
	IDirect3DDevice8(IDirect3D8 *pD3D, IDirect3DDevice9 *pProxyDevice) : mD3D(pD3D), mProxy(pProxyDevice), mBaseVertexIndex(0)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE								AddRef(void);
	virtual ULONG STDMETHODCALLTYPE								Release(void);

	virtual HRESULT STDMETHODCALLTYPE							TestCooperativeLevel(void);
	virtual UINT STDMETHODCALLTYPE								GetAvailableTextureMem(void);
	virtual HRESULT STDMETHODCALLTYPE							ResourceManagerDiscardBytes(DWORD Bytes);
	virtual HRESULT STDMETHODCALLTYPE							GetDirect3D(IDirect3D8 **ppD3D8);
	virtual HRESULT STDMETHODCALLTYPE							GetDeviceCaps(D3DCAPS8 *pCaps);
	virtual HRESULT STDMETHODCALLTYPE							GetDisplayMode(D3DDISPLAYMODE *pMode);
	virtual HRESULT STDMETHODCALLTYPE							GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
	virtual HRESULT STDMETHODCALLTYPE							SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8 *pCursorBitmap);
	virtual void STDMETHODCALLTYPE								SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags);
	virtual BOOL STDMETHODCALLTYPE								ShowCursor(BOOL bShow);
	virtual HRESULT STDMETHODCALLTYPE							CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS8 *pPresentationParameters, IDirect3DSwapChain8 **ppSwapChain);
	virtual HRESULT STDMETHODCALLTYPE							Reset(D3DPRESENT_PARAMETERS8 *pPresentationParameters);
	virtual HRESULT STDMETHODCALLTYPE							Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);
	virtual HRESULT STDMETHODCALLTYPE							GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer);
	virtual HRESULT STDMETHODCALLTYPE							GetRasterStatus(D3DRASTER_STATUS *pRasterStatus);
	virtual void STDMETHODCALLTYPE								SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp);
	virtual void STDMETHODCALLTYPE								GetGammaRamp(D3DGAMMARAMP *pRamp);
	virtual HRESULT STDMETHODCALLTYPE							CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8 **ppTexture);
	virtual HRESULT STDMETHODCALLTYPE							CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8 **ppVolumeTexture);
	virtual HRESULT STDMETHODCALLTYPE							CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture);
	virtual HRESULT STDMETHODCALLTYPE							CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8 **ppVertexBuffer);
	virtual HRESULT STDMETHODCALLTYPE							CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8 **ppIndexBuffer);
	virtual HRESULT STDMETHODCALLTYPE							CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8 **ppSurface);
	virtual HRESULT STDMETHODCALLTYPE							CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8 **ppSurface);
	virtual HRESULT STDMETHODCALLTYPE							CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8 **ppSurface);
	virtual HRESULT STDMETHODCALLTYPE							CopyRects(IDirect3DSurface8 *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, IDirect3DSurface8 *pDestinationSurface, CONST POINT *pDestPointsArray);
	virtual HRESULT STDMETHODCALLTYPE							UpdateTexture(IDirect3DBaseTexture8 *pSourceTexture, IDirect3DBaseTexture8 *pDestinationTexture);
	virtual HRESULT STDMETHODCALLTYPE							GetFrontBuffer(IDirect3DSurface8 *pDestSurface);
	virtual HRESULT STDMETHODCALLTYPE							SetRenderTarget(IDirect3DSurface8 *pRenderTarget, IDirect3DSurface8 *pNewZStencil);
	virtual HRESULT STDMETHODCALLTYPE							GetRenderTarget(IDirect3DSurface8 **ppRenderTarget);
	virtual HRESULT STDMETHODCALLTYPE							GetDepthStencilSurface(IDirect3DSurface8 **ppZStencilSurface);
	virtual HRESULT STDMETHODCALLTYPE							BeginScene(void);
	virtual HRESULT STDMETHODCALLTYPE							EndScene(void);
	virtual HRESULT STDMETHODCALLTYPE							Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	virtual HRESULT STDMETHODCALLTYPE							SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
	virtual HRESULT STDMETHODCALLTYPE							GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix);
	virtual HRESULT STDMETHODCALLTYPE							MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
	virtual HRESULT STDMETHODCALLTYPE							SetViewport(CONST D3DVIEWPORT8 *pViewport);
	virtual HRESULT STDMETHODCALLTYPE							GetViewport(D3DVIEWPORT8 *pViewport);
	virtual HRESULT STDMETHODCALLTYPE							SetMaterial(CONST D3DMATERIAL8 *pMaterial);
	virtual HRESULT STDMETHODCALLTYPE							GetMaterial(D3DMATERIAL8 *pMaterial);
	virtual HRESULT STDMETHODCALLTYPE							SetLight(DWORD Index, CONST D3DLIGHT8 *pLight);
	virtual HRESULT STDMETHODCALLTYPE							GetLight(DWORD Index, D3DLIGHT8 *pLight);
	virtual HRESULT STDMETHODCALLTYPE							LightEnable(DWORD Index, BOOL Enable);
	virtual HRESULT STDMETHODCALLTYPE							GetLightEnable(DWORD Index, BOOL *pEnable);
	virtual HRESULT STDMETHODCALLTYPE							SetClipPlane(DWORD Index, CONST float *pPlane);
	virtual HRESULT STDMETHODCALLTYPE							GetClipPlane(DWORD Index, float *pPlane);
	virtual HRESULT STDMETHODCALLTYPE							SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	virtual HRESULT STDMETHODCALLTYPE							GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue);
	virtual HRESULT STDMETHODCALLTYPE							BeginStateBlock(void);
	virtual HRESULT STDMETHODCALLTYPE							EndStateBlock(DWORD *pToken);
	virtual HRESULT STDMETHODCALLTYPE							ApplyStateBlock(DWORD Token);
	virtual HRESULT STDMETHODCALLTYPE							CaptureStateBlock(DWORD Token);
	virtual HRESULT STDMETHODCALLTYPE							DeleteStateBlock(DWORD Token);
	virtual HRESULT STDMETHODCALLTYPE							CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD *pToken);
	virtual HRESULT STDMETHODCALLTYPE							SetClipStatus(CONST D3DCLIPSTATUS8 *pClipStatus);
	virtual HRESULT STDMETHODCALLTYPE							GetClipStatus(D3DCLIPSTATUS8 *pClipStatus);
	virtual HRESULT STDMETHODCALLTYPE							GetTexture(DWORD Stage, IDirect3DBaseTexture8 **ppTexture);
	virtual HRESULT STDMETHODCALLTYPE							SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture);
	virtual HRESULT STDMETHODCALLTYPE							GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue);
	virtual HRESULT STDMETHODCALLTYPE							SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	virtual HRESULT STDMETHODCALLTYPE							ValidateDevice(DWORD *pNumPasses);
	virtual HRESULT STDMETHODCALLTYPE							GetInfo(DWORD DevInfoID, void *pDevInfoStruct, DWORD DevInfoStructSize);
	virtual HRESULT STDMETHODCALLTYPE							SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries);
	virtual HRESULT STDMETHODCALLTYPE							GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries);
	virtual HRESULT STDMETHODCALLTYPE							SetCurrentTexturePalette(UINT PaletteNumber);
	virtual HRESULT STDMETHODCALLTYPE							GetCurrentTexturePalette(UINT *PaletteNumber);
	virtual HRESULT STDMETHODCALLTYPE							DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	virtual HRESULT STDMETHODCALLTYPE							DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
	virtual HRESULT STDMETHODCALLTYPE							DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
	virtual HRESULT STDMETHODCALLTYPE							DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
	virtual HRESULT STDMETHODCALLTYPE							ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8 *pDestBuffer, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							CreateVertexShader(CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage);
	virtual HRESULT STDMETHODCALLTYPE							SetVertexShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE							GetVertexShader(DWORD *pHandle);
	virtual HRESULT STDMETHODCALLTYPE							DeleteVertexShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE							SetVertexShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE							GetVertexShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE							GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE							GetVertexShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE							SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 *pStreamData, UINT Stride);
	virtual HRESULT STDMETHODCALLTYPE							GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 **ppStreamData, UINT *pStride);
	virtual HRESULT STDMETHODCALLTYPE							SetIndices(IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex);
	virtual HRESULT STDMETHODCALLTYPE							GetIndices(IDirect3DIndexBuffer8 **ppIndexData, UINT *pBaseVertexIndex);
	virtual HRESULT STDMETHODCALLTYPE							CreatePixelShader(CONST DWORD *pFunction, DWORD *pHandle);
	virtual HRESULT STDMETHODCALLTYPE							SetPixelShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE							GetPixelShader(DWORD *pHandle);
	virtual HRESULT STDMETHODCALLTYPE							DeletePixelShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE							SetPixelShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE							GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE							GetPixelShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE							DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo);
	virtual HRESULT STDMETHODCALLTYPE							DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo);
	virtual HRESULT STDMETHODCALLTYPE							DeletePatch(UINT Handle);

	IDirect3D8 *												mD3D;
	IDirect3DDevice9 *											mProxy;
	INT															mBaseVertexIndex;
	std::vector<IDirect3DStateBlock9 *>							mStateBlocks;
	std::vector<std::pair<IDirect3DVertexShader9 *, IDirect3DVertexDeclaration9 *>> mVertexShaders;
	std::vector<std::pair<DWORD *, std::size_t>>				mVertexShaderDeclarations;
	std::vector<IDirect3DPixelShader9 *>						mPixelShaders;
};
struct															IDirect3DResource8 : public IUnknown
{
	IDirect3DResource8(IDirect3DDevice8 *pDevice, IDirect3DResource9 *pProxyResource) : mRef(1), mDevice(pDevice), mProxy(pProxyResource)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj) = 0;
	virtual ULONG STDMETHODCALLTYPE								AddRef(void);
	virtual ULONG STDMETHODCALLTYPE								Release(void);

	virtual HRESULT STDMETHODCALLTYPE							GetDevice(IDirect3DDevice8 **ppDevice);
	virtual HRESULT STDMETHODCALLTYPE							SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE							FreePrivateData(REFGUID refguid);
	virtual DWORD STDMETHODCALLTYPE								SetPriority(DWORD PriorityNew);
	virtual DWORD STDMETHODCALLTYPE								GetPriority(void);
	virtual void STDMETHODCALLTYPE								PreLoad(void);
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE					GetType(void);

	ULONG														mRef;
	IDirect3DDevice8 *											mDevice;
	IDirect3DResource9 *										mProxy;
};
struct															IDirect3DBaseTexture8 : public IDirect3DResource8
{
	IDirect3DBaseTexture8(IDirect3DDevice8 *pDevice, IDirect3DBaseTexture9 *pProxyTexture) : IDirect3DResource8(pDevice, pProxyTexture)
	{
	}

	virtual DWORD STDMETHODCALLTYPE								SetLOD(DWORD LODNew);
	virtual DWORD STDMETHODCALLTYPE								GetLOD(void);
	virtual DWORD STDMETHODCALLTYPE								GetLevelCount(void);
};
struct															IDirect3DTexture8 : public IDirect3DBaseTexture8
{
	IDirect3DTexture8(IDirect3DDevice8 *pDevice, IDirect3DTexture9 *pProxyTexture) : IDirect3DBaseTexture8(pDevice, pProxyTexture)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);

	virtual HRESULT STDMETHODCALLTYPE							GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc);
	virtual HRESULT STDMETHODCALLTYPE							GetSurfaceLevel(UINT Level, IDirect3DSurface8 **ppSurfaceLevel);
	virtual HRESULT STDMETHODCALLTYPE							LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							UnlockRect(UINT Level);
	virtual HRESULT STDMETHODCALLTYPE							AddDirtyRect(CONST RECT *pDirtyRect);
};
struct															IDirect3DVolumeTexture8 : public IDirect3DBaseTexture8
{
	IDirect3DVolumeTexture8(IDirect3DDevice8 *pDevice, IDirect3DVolumeTexture9 *pProxyTexture) : IDirect3DBaseTexture8(pDevice, pProxyTexture)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);

	virtual HRESULT STDMETHODCALLTYPE							GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc);
	virtual HRESULT STDMETHODCALLTYPE							GetVolumeLevel(UINT Level, IDirect3DVolume8 **ppVolumeLevel);
	virtual HRESULT STDMETHODCALLTYPE							LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							UnlockBox(UINT Level);
	virtual HRESULT STDMETHODCALLTYPE							AddDirtyBox(CONST D3DBOX *pDirtyBox);
};
struct															IDirect3DCubeTexture8 : public IDirect3DBaseTexture8
{
	IDirect3DCubeTexture8(IDirect3DDevice8 *pDevice, IDirect3DCubeTexture9 *pProxyBuffer) : IDirect3DBaseTexture8(pDevice, pProxyBuffer)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);

	virtual HRESULT STDMETHODCALLTYPE							GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc);
	virtual HRESULT STDMETHODCALLTYPE							GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface8 **ppCubeMapSurface);
	virtual HRESULT STDMETHODCALLTYPE							LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level);
	virtual HRESULT STDMETHODCALLTYPE							AddDirtyRect(D3DCUBEMAP_FACES FaceType, CONST RECT *pDirtyRect);
};
struct															IDirect3DVertexBuffer8 : public IDirect3DResource8
{
	IDirect3DVertexBuffer8(IDirect3DDevice8 *pDevice, IDirect3DVertexBuffer9 *pProxyBuffer) : IDirect3DResource8(pDevice, pProxyBuffer)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);

	virtual HRESULT STDMETHODCALLTYPE							Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							Unlock(void);
	virtual HRESULT STDMETHODCALLTYPE							GetDesc(D3DVERTEXBUFFER_DESC *pDesc);
};
struct															IDirect3DIndexBuffer8 : public IDirect3DResource8
{
	IDirect3DIndexBuffer8(IDirect3DDevice8 *pDevice, IDirect3DIndexBuffer9 *pProxyBuffer) : IDirect3DResource8(pDevice, pProxyBuffer)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);

	virtual HRESULT STDMETHODCALLTYPE							Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							Unlock(void);
	virtual HRESULT STDMETHODCALLTYPE							GetDesc(D3DINDEXBUFFER_DESC *pDesc);
};
struct															IDirect3DSurface8 : public IUnknown
{
	IDirect3DSurface8(IDirect3DDevice8 *pDevice, IDirect3DSurface9 *pProxyBuffer) : mRef(1), mDevice(pDevice), mProxy(pProxyBuffer)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE								AddRef(void);
	virtual ULONG STDMETHODCALLTYPE								Release(void);

	virtual HRESULT STDMETHODCALLTYPE							GetDevice(IDirect3DDevice8 **ppDevice);
	virtual HRESULT STDMETHODCALLTYPE							SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE							FreePrivateData(REFGUID refguid);
	virtual HRESULT STDMETHODCALLTYPE							GetContainer(REFIID riid, void **ppContainer);
	virtual HRESULT STDMETHODCALLTYPE							GetDesc(D3DSURFACE_DESC *pDesc);
	virtual HRESULT STDMETHODCALLTYPE							LockRect(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							UnlockRect(void);

	ULONG														mRef;
	IDirect3DDevice8 *											mDevice;
	IDirect3DSurface9 *											mProxy;
};
struct															IDirect3DVolume8 : public IUnknown
{
	IDirect3DVolume8(IDirect3DDevice8 *pDevice, IDirect3DVolume9 *pProxyBuffer) : mRef(1), mDevice(pDevice), mProxy(pProxyBuffer)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE								AddRef(void);
	virtual ULONG STDMETHODCALLTYPE								Release(void);

	virtual HRESULT STDMETHODCALLTYPE							GetDevice(IDirect3DDevice8 **ppDevice);
	virtual HRESULT STDMETHODCALLTYPE							SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE							FreePrivateData(REFGUID refguid);
	virtual HRESULT STDMETHODCALLTYPE							GetContainer(REFIID riid, void **ppContainer);
	virtual HRESULT STDMETHODCALLTYPE							GetDesc(D3DVOLUME_DESC *pDesc);
	virtual HRESULT STDMETHODCALLTYPE							LockBox(D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE							UnlockBox(void);

	ULONG														mRef;
	IDirect3DDevice8 *											mDevice;
	IDirect3DVolume9 *											mProxy;
};
struct															IDirect3DSwapChain8 : public IUnknown
{
	IDirect3DSwapChain8(IDirect3DDevice8 *pDevice, IDirect3DSwapChain9 *pProxySwapChain) : mRef(1), mDevice(pDevice), mProxy(pProxySwapChain)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE							QueryInterface(REFIID riid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE								AddRef(void);
	virtual ULONG STDMETHODCALLTYPE								Release(void);

	virtual HRESULT STDMETHODCALLTYPE							Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);
	virtual HRESULT STDMETHODCALLTYPE							GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer);

	ULONG														mRef;
	IDirect3DDevice8 *											mDevice;
	IDirect3DSwapChain9 *										mProxy;
};

DEFINE_GUID(IID_IDirect3D8, 0x1DD9E8DA, 0x1C77, 0x4D40, 0xB0, 0xCF, 0x98, 0xFE, 0xFD, 0xFF, 0x95, 0x12);
DEFINE_GUID(IID_IDirect3DDevice8, 0x7385E5DF, 0x8FE8, 0x41D5, 0x86, 0xB6, 0xD7, 0xB4, 0x85, 0x47, 0xB6, 0xCF);
DEFINE_GUID(IID_IDirect3DResource8, 0x1B36BB7B, 0x9B7, 0x410A, 0xB4, 0x45, 0x7D, 0x14, 0x30, 0xD7, 0xB3, 0x3F);
DEFINE_GUID(IID_IDirect3DBaseTexture8, 0xB4211CFA, 0x51B9, 0x4A9F, 0xAB, 0x78, 0xDB, 0x99, 0xB2, 0xBB, 0x67, 0x8E);
DEFINE_GUID(IID_IDirect3DTexture8, 0xE4CDD575, 0x2866, 0x4F01, 0xB1, 0x2E, 0x7E, 0xEC, 0xE1, 0xEC, 0x93, 0x58);
DEFINE_GUID(IID_IDirect3DVolumeTexture8, 0x4B8AAAFA, 0x140F, 0x42BA, 0x91, 0x31, 0x59, 0x7E, 0xAF, 0xAA, 0x2E, 0xAD);
DEFINE_GUID(IID_IDirect3DCubeTexture8, 0x3EE5B968, 0x2ACA, 0x4C34, 0x8B, 0xB5, 0x7E, 0x0C, 0x3D, 0x19, 0xB7, 0x50);
DEFINE_GUID(IID_IDirect3DVertexBuffer8, 0x8AEEEAC7, 0x05F9, 0x44D4, 0xB5, 0x91, 0x00, 0x0B, 0x0D, 0xF1, 0xCB, 0x95);
DEFINE_GUID(IID_IDirect3DIndexBuffer8, 0x0E689C9A, 0x053D, 0x44A0, 0x9D, 0x92, 0xDB, 0x0E, 0x3D, 0x75, 0x0F, 0x86);
DEFINE_GUID(IID_IDirect3DSurface8, 0xb96EEBCA, 0xB326, 0x4EA5, 0x88, 0x2F, 0x2F, 0xF5, 0xBA, 0xE0, 0x21, 0xDD);
DEFINE_GUID(IID_IDirect3DVolume8, 0xBD7349F5, 0x14F1, 0x42E4, 0x9C, 0x79, 0x97, 0x23, 0x80, 0xDB, 0x40, 0xC0);
DEFINE_GUID(IID_IDirect3DSwapChain8, 0x928C088B, 0x76B9, 0x4C6B, 0xA5, 0x36, 0xA5, 0x90, 0x85, 0x38, 0x76, 0xCD);

// -----------------------------------------------------------------------------------------------------

// IDirect3DResource8
ULONG STDMETHODCALLTYPE											IDirect3DResource8::AddRef(void)
{
	++this->mRef;

	return this->mProxy->AddRef();
}
ULONG STDMETHODCALLTYPE											IDirect3DResource8::Release(void)
{
	const ULONG ref = this->mProxy->Release();

	if (--this->mRef == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE										IDirect3DResource8::GetDevice(IDirect3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	this->mDevice->AddRef();

	*ppDevice = this->mDevice;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DResource8::SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags)
{
	return this->mProxy->SetPrivateData(refguid, pData, SizeOfData, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DResource8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return this->mProxy->GetPrivateData(refguid, pData, pSizeOfData);
}
HRESULT STDMETHODCALLTYPE										IDirect3DResource8::FreePrivateData(REFGUID refguid)
{
	return this->mProxy->FreePrivateData(refguid);
}
DWORD STDMETHODCALLTYPE											IDirect3DResource8::SetPriority(DWORD PriorityNew)
{
	return this->mProxy->SetPriority(PriorityNew);
}
DWORD STDMETHODCALLTYPE											IDirect3DResource8::GetPriority(void)
{
	return this->mProxy->GetPriority();
}
void STDMETHODCALLTYPE											IDirect3DResource8::PreLoad(void)
{
	this->mProxy->PreLoad();
}
D3DRESOURCETYPE STDMETHODCALLTYPE								IDirect3DResource8::GetType(void)
{
	return this->mProxy->GetType();
}

// IDirect3DBaseTexture8
DWORD STDMETHODCALLTYPE											IDirect3DBaseTexture8::SetLOD(DWORD LODNew)
{
	return static_cast<IDirect3DBaseTexture9 *>(this->mProxy)->SetLOD(LODNew);
}
DWORD STDMETHODCALLTYPE											IDirect3DBaseTexture8::GetLOD(void)
{
	return static_cast<IDirect3DBaseTexture9 *>(this->mProxy)->GetLOD();
}
DWORD STDMETHODCALLTYPE											IDirect3DBaseTexture8::GetLevelCount(void)
{
	return static_cast<IDirect3DBaseTexture9 *>(this->mProxy)->GetLevelCount();
}

// IDirect3DTexture8
HRESULT STDMETHODCALLTYPE										IDirect3DTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DBaseTexture8 || riid == IID_IDirect3DTexture8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc)
{
	return static_cast<IDirect3DTexture9 *>(this->mProxy)->GetLevelDesc(Level, pDesc);
}
HRESULT STDMETHODCALLTYPE										IDirect3DTexture8::GetSurfaceLevel(UINT Level, IDirect3DSurface8 **ppSurfaceLevel)
{
	if (ppSurfaceLevel == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = static_cast<IDirect3DTexture9 *>(this->mProxy)->GetSurfaceLevel(Level, &surface);

	if (SUCCEEDED(hr) && surface != nullptr)
	{
		*ppSurfaceLevel = new IDirect3DSurface8(this->mDevice, surface);
	}
	else
	{
		*ppSurfaceLevel = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DTexture8::LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags)
{
	return static_cast<IDirect3DTexture9 *>(this->mProxy)->LockRect(Level, pLockedRect, pRect, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DTexture8::UnlockRect(UINT Level)
{
	return static_cast<IDirect3DTexture9 *>(this->mProxy)->UnlockRect(Level);
}
HRESULT STDMETHODCALLTYPE										IDirect3DTexture8::AddDirtyRect(CONST RECT *pDirtyRect)
{
	return static_cast<IDirect3DTexture9 *>(this->mProxy)->AddDirtyRect(pDirtyRect);
}

// IDirect3DVolumeTexture8
HRESULT STDMETHODCALLTYPE										IDirect3DVolumeTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DBaseTexture8 || riid == IID_IDirect3DVolumeTexture8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolumeTexture8::GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc)
{
	return static_cast<IDirect3DVolumeTexture9 *>(this->mProxy)->GetLevelDesc(Level, pDesc);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolumeTexture8::GetVolumeLevel(UINT Level, IDirect3DVolume8 **ppVolumeLevel)
{
	if (ppVolumeLevel == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DVolume9 *volume = nullptr;

	HRESULT hr = static_cast<IDirect3DVolumeTexture9 *>(this->mProxy)->GetVolumeLevel(Level, &volume);

	if (SUCCEEDED(hr) && volume != nullptr)
	{
		*ppVolumeLevel = new IDirect3DVolume8(this->mDevice, volume);
	}
	else
	{
		*ppVolumeLevel = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolumeTexture8::LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags)
{
	return static_cast<IDirect3DVolumeTexture9 *>(this->mProxy)->LockBox(Level, pLockedVolume, pBox, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolumeTexture8::UnlockBox(UINT Level)
{
	return static_cast<IDirect3DVolumeTexture9 *>(this->mProxy)->UnlockBox(Level);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolumeTexture8::AddDirtyBox(CONST D3DBOX *pDirtyBox)
{
	return static_cast<IDirect3DVolumeTexture9 *>(this->mProxy)->AddDirtyBox(pDirtyBox);
}

// IDirect3DCubeTexture8
HRESULT STDMETHODCALLTYPE										IDirect3DCubeTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DBaseTexture8 || riid == IID_IDirect3DCubeTexture8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DCubeTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc)
{
	return static_cast<IDirect3DCubeTexture9 *>(this->mProxy)->GetLevelDesc(Level, pDesc);
}
HRESULT STDMETHODCALLTYPE										IDirect3DCubeTexture8::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface8 **ppCubeMapSurface)
{
	if (ppCubeMapSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = static_cast<IDirect3DCubeTexture9 *>(this->mProxy)->GetCubeMapSurface(FaceType, Level, &surface);

	if (SUCCEEDED(hr) && surface != nullptr)
	{
		*ppCubeMapSurface = new IDirect3DSurface8(this->mDevice, surface);
	}
	else
	{
		*ppCubeMapSurface = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DCubeTexture8::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags)
{
	return static_cast<IDirect3DCubeTexture9 *>(this->mProxy)->LockRect(FaceType, Level, pLockedRect, pRect, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DCubeTexture8::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level)
{
	return static_cast<IDirect3DCubeTexture9 *>(this->mProxy)->UnlockRect(FaceType, Level);
}
HRESULT STDMETHODCALLTYPE										IDirect3DCubeTexture8::AddDirtyRect(D3DCUBEMAP_FACES FaceType, CONST RECT *pDirtyRect)
{
	return static_cast<IDirect3DCubeTexture9 *>(this->mProxy)->AddDirtyRect(FaceType, pDirtyRect);
}

// IDirect3DVertexBuffer8
HRESULT STDMETHODCALLTYPE										IDirect3DVertexBuffer8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DVertexBuffer8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DVertexBuffer8::Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags)
{
	return static_cast<IDirect3DVertexBuffer9 *>(this->mProxy)->Lock(OffsetToLock, SizeToLock, reinterpret_cast<void **>(ppbData), Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVertexBuffer8::Unlock(void)
{
	return static_cast<IDirect3DVertexBuffer9 *>(this->mProxy)->Unlock();
}
HRESULT STDMETHODCALLTYPE										IDirect3DVertexBuffer8::GetDesc(D3DVERTEXBUFFER_DESC *pDesc)
{
	return static_cast<IDirect3DVertexBuffer9 *>(this->mProxy)->GetDesc(pDesc);
}

// IDirect3DIndexBuffer8
HRESULT STDMETHODCALLTYPE										IDirect3DIndexBuffer8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DResource8 || riid == IID_IDirect3DIndexBuffer8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DIndexBuffer8::Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags)
{
	return static_cast<IDirect3DIndexBuffer9 *>(this->mProxy)->Lock(OffsetToLock, SizeToLock, reinterpret_cast<void **>(ppbData), Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DIndexBuffer8::Unlock(void)
{
	return static_cast<IDirect3DIndexBuffer9 *>(this->mProxy)->Unlock();
}
HRESULT STDMETHODCALLTYPE										IDirect3DIndexBuffer8::GetDesc(D3DINDEXBUFFER_DESC *pDesc)
{
	return static_cast<IDirect3DIndexBuffer9 *>(this->mProxy)->GetDesc(pDesc);
}

// IDirect3DSurface8
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DSurface8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
ULONG STDMETHODCALLTYPE											IDirect3DSurface8::AddRef(void)
{
	++this->mRef;

	return this->mProxy->AddRef();
}
ULONG STDMETHODCALLTYPE											IDirect3DSurface8::Release(void)
{
	const ULONG ref = this->mProxy->Release();

	if (--this->mRef == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::GetDevice(IDirect3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	this->mDevice->AddRef();

	*ppDevice = this->mDevice;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags)
{
	return this->mProxy->SetPrivateData(refguid, pData, SizeOfData, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return this->mProxy->GetPrivateData(refguid, pData, pSizeOfData);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::FreePrivateData(REFGUID refguid)
{
	return this->mProxy->FreePrivateData(refguid);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::GetContainer(REFIID riid, void **ppContainer)
{
	return this->mProxy->GetContainer(riid, ppContainer);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::GetDesc(D3DSURFACE_DESC *pDesc)
{
	return this->mProxy->GetDesc(pDesc);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::LockRect(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags)
{
	return this->mProxy->LockRect(pLockedRect, pRect, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSurface8::UnlockRect(void)
{
	return this->mProxy->UnlockRect();
}

// IDirect3DVolume8
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DVolume8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
ULONG STDMETHODCALLTYPE											IDirect3DVolume8::AddRef(void)
{
	++this->mRef;

	return this->mProxy->AddRef();
}
ULONG STDMETHODCALLTYPE											IDirect3DVolume8::Release(void)
{
	const ULONG ref = this->mProxy->Release();

	if (--this->mRef == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::GetDevice(IDirect3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	this->mDevice->AddRef();

	*ppDevice = this->mDevice;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags)
{
	return this->mProxy->SetPrivateData(refguid, pData, SizeOfData, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return this->mProxy->GetPrivateData(refguid, pData, pSizeOfData);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::FreePrivateData(REFGUID refguid)
{
	return this->mProxy->FreePrivateData(refguid);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::GetContainer(REFIID riid, void **ppContainer)
{
	return this->mProxy->GetContainer(riid, ppContainer);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::GetDesc(D3DVOLUME_DESC *pDesc)
{
	return this->mProxy->GetDesc(pDesc);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::LockBox(D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags)
{
	return this->mProxy->LockBox(pLockedVolume, pBox, Flags);
}
HRESULT STDMETHODCALLTYPE										IDirect3DVolume8::UnlockBox(void)
{
	return this->mProxy->UnlockBox();
}

// IDirect3DSwapChain8
HRESULT STDMETHODCALLTYPE										IDirect3DSwapChain8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DSwapChain8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else if (riid == IID_IDirect3DDevice8)
	{
		this->mDevice->AddRef();

		*ppvObj = this->mDevice;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
ULONG STDMETHODCALLTYPE											IDirect3DSwapChain8::AddRef(void)
{
	++this->mRef;

	return this->mProxy->AddRef();
}
ULONG STDMETHODCALLTYPE											IDirect3DSwapChain8::Release(void)
{
	const ULONG ref = this->mProxy->Release();

	if (--this->mRef == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE										IDirect3DSwapChain8::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	return this->mProxy->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, 0);
}
HRESULT STDMETHODCALLTYPE										IDirect3DSwapChain8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer)
{
	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->GetBackBuffer(iBackBuffer, Type, &surface);

	if (SUCCEEDED(hr) && surface != nullptr)
	{
		*ppBackBuffer = new IDirect3DSurface8(this->mDevice, surface);
	}
	else
	{
		*ppBackBuffer = nullptr;
	}
		
	return hr;
}

// IDirect3DDevice8
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3DDevice8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else if (riid == IID_IDirect3D8)
	{
		return GetDirect3D(reinterpret_cast<IDirect3D8 **>(ppvObj));
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
ULONG STDMETHODCALLTYPE											IDirect3DDevice8::AddRef(void)
{
	return this->mProxy->AddRef();
}
ULONG STDMETHODCALLTYPE											IDirect3DDevice8::Release(void)
{
	const ULONG ref = this->mProxy->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::TestCooperativeLevel(void)
{
	return this->mProxy->TestCooperativeLevel();
}
UINT STDMETHODCALLTYPE											IDirect3DDevice8::GetAvailableTextureMem(void)
{
	return this->mProxy->GetAvailableTextureMem();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::ResourceManagerDiscardBytes(DWORD Bytes)
{
	UNREFERENCED_PARAMETER(Bytes);

	return this->mProxy->EvictManagedResources();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetDirect3D(IDirect3D8 **ppD3D8)
{
	if (ppD3D8 == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	this->mD3D->AddRef();

	*ppD3D8 = this->mD3D;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetDeviceCaps(D3DCAPS8 *pCaps)
{
	if (pCaps == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DCAPS9 caps;

	HRESULT hr = this->mProxy->GetDeviceCaps(&caps);

	if (SUCCEEDED(hr))
	{
		std::memcpy(pCaps, &caps, sizeof(D3DCAPS8));
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
	return this->mProxy->GetDisplayMode(0, pMode);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return this->mProxy->GetCreationParameters(pParameters);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8 *pCursorBitmap)
{
	if (pCursorBitmap == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mProxy->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap->mProxy);
}
void STDMETHODCALLTYPE											IDirect3DDevice8::SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags)
{
	this->mProxy->SetCursorPosition(XScreenSpace, YScreenSpace, Flags);
}
BOOL STDMETHODCALLTYPE											IDirect3DDevice8::ShowCursor(BOOL bShow)
{
	return this->mProxy->ShowCursor(bShow);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS8 *pPresentationParameters, IDirect3DSwapChain8 **ppSwapChain)
{
	LOG(INFO) << "Redirecting '" << "IDirect3DDevice8::CreateAdditionalSwapChain" << "(" << this << ", " << pPresentationParameters << ", " << ppSwapChain << ")' ...";

	if (ppSwapChain == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DPRESENT_PARAMETERS pp;
	pp.BackBufferWidth = pPresentationParameters->BackBufferWidth;
	pp.BackBufferHeight = pPresentationParameters->BackBufferHeight;
	pp.BackBufferFormat = pPresentationParameters->BackBufferFormat;
	pp.BackBufferCount = pPresentationParameters->BackBufferCount;
	pp.MultiSampleType = pPresentationParameters->MultiSampleType;
	pp.MultiSampleQuality = 0;
	pp.SwapEffect = pPresentationParameters->SwapEffect;
	pp.hDeviceWindow = pPresentationParameters->hDeviceWindow;
	pp.Windowed = pPresentationParameters->Windowed;
	pp.EnableAutoDepthStencil = pPresentationParameters->EnableAutoDepthStencil;
	pp.AutoDepthStencilFormat = pPresentationParameters->AutoDepthStencilFormat;
	pp.Flags = pPresentationParameters->Flags;
	pp.FullScreen_RefreshRateInHz = pPresentationParameters->FullScreen_RefreshRateInHz;
	pp.PresentationInterval = pPresentationParameters->FullScreen_PresentationInterval;

#define D3DSWAPEFFECT_COPY_VSYNC 4
#define D3DPRESENT_RATE_UNLIMITED 0x7FFFFFFF

	if (pp.SwapEffect == D3DSWAPEFFECT_COPY_VSYNC)
	{
		LOG(WARNING) << "> 'IDirect3DDevice8::CreateAdditionalSwapChain' failed because 'D3DSWAPEFFECT_COPY_VSYNC' is not supported!";

		*ppSwapChain = nullptr;

		return D3DERR_NOTAVAILABLE;
	}
	if (pp.PresentationInterval == D3DPRESENT_RATE_UNLIMITED)
	{
		pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	IDirect3DSwapChain9 *swapchain = nullptr;

	HRESULT hr = this->mProxy->CreateAdditionalSwapChain(&pp, &swapchain);

	if (SUCCEEDED(hr))
	{
		*ppSwapChain = new IDirect3DSwapChain8(this, swapchain);
	}
	else
	{
		*ppSwapChain = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::Reset(D3DPRESENT_PARAMETERS8 *pPresentationParameters)
{
	LOG(INFO) << "Redirecting '" << "IDirect3DDevice8::Reset" << "(" << this << ", " << pPresentationParameters << ")' ...";

	D3DPRESENT_PARAMETERS pp;
	pp.BackBufferWidth = pPresentationParameters->BackBufferWidth;
	pp.BackBufferHeight = pPresentationParameters->BackBufferHeight;
	pp.BackBufferFormat = pPresentationParameters->BackBufferFormat;
	pp.BackBufferCount = pPresentationParameters->BackBufferCount;
	pp.MultiSampleType = pPresentationParameters->MultiSampleType;
	pp.MultiSampleQuality = 0;
	pp.SwapEffect = pPresentationParameters->SwapEffect;
	pp.hDeviceWindow = pPresentationParameters->hDeviceWindow;
	pp.Windowed = pPresentationParameters->Windowed;
	pp.EnableAutoDepthStencil = pPresentationParameters->EnableAutoDepthStencil;
	pp.AutoDepthStencilFormat = pPresentationParameters->AutoDepthStencilFormat;
	pp.Flags = pPresentationParameters->Flags;
	pp.FullScreen_RefreshRateInHz = pPresentationParameters->FullScreen_RefreshRateInHz;
	pp.PresentationInterval = pPresentationParameters->FullScreen_PresentationInterval;

	if (pp.SwapEffect == D3DSWAPEFFECT_COPY_VSYNC)
	{
		LOG(WARNING) << "> 'IDirect3DDevice8::Reset' failed because 'D3DSWAPEFFECT_COPY_VSYNC' is not supported!";

		return D3DERR_NOTAVAILABLE;
	}
	if (pp.PresentationInterval == D3DPRESENT_RATE_UNLIMITED)
	{
		pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	return this->mProxy->Reset(&pp);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	return this->mProxy->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer)
{
	if (ppBackBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->GetBackBuffer(0, iBackBuffer, Type, &surface);

	if (SUCCEEDED(hr) && surface != nullptr)
	{
		*ppBackBuffer = new IDirect3DSurface8(this, surface);
	}
	else
	{
		*ppBackBuffer = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
	return this->mProxy->GetRasterStatus(0, pRasterStatus);
}
void STDMETHODCALLTYPE											IDirect3DDevice8::SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp)
{
	this->mProxy->SetGammaRamp(0, Flags, pRamp);
}
void STDMETHODCALLTYPE											IDirect3DDevice8::GetGammaRamp(D3DGAMMARAMP *pRamp)
{
	this->mProxy->GetGammaRamp(0, pRamp);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8 **ppTexture)
{
	if (ppTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DTexture9 *texture = nullptr;

	HRESULT hr = this->mProxy->CreateTexture(Width, Height, Levels, Usage, Format, Pool, &texture, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppTexture = new IDirect3DTexture8(this, texture);
	}
	else
	{
		*ppTexture = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8 **ppVolumeTexture)
{
	if (ppVolumeTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DVolumeTexture9 *texture = nullptr;

	HRESULT hr = this->mProxy->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, &texture, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppVolumeTexture = new IDirect3DVolumeTexture8(this, texture);
	}
	else
	{
		*ppVolumeTexture = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture)
{
	if (ppCubeTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DCubeTexture9 *texture = nullptr;

	HRESULT hr = this->mProxy->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, &texture, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppCubeTexture = new IDirect3DCubeTexture8(this, texture);
	}
	else
	{
		*ppCubeTexture = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8 **ppVertexBuffer)
{
	if (ppVertexBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DVertexBuffer9 *buffer = nullptr;

	HRESULT hr = this->mProxy->CreateVertexBuffer(Length, Usage, FVF, Pool, &buffer, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppVertexBuffer = new IDirect3DVertexBuffer8(this, buffer);
	}
	else
	{
		*ppVertexBuffer = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8 **ppIndexBuffer)
{
	if (ppIndexBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DIndexBuffer9 *buffer = nullptr;

	HRESULT hr = this->mProxy->CreateIndexBuffer(Length, Usage, Format, Pool, &buffer, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppIndexBuffer = new IDirect3DIndexBuffer8(this, buffer);
	}
	else
	{
		*ppIndexBuffer = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->CreateRenderTarget(Width, Height, Format, MultiSample, 0, Lockable, &surface, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppSurface = new IDirect3DSurface8(this, surface);
	}
	else
	{
		*ppSurface = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->CreateDepthStencilSurface(Width, Height, Format, MultiSample, 0, FALSE, &surface, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppSurface = new IDirect3DSurface8(this, surface);
	}
	else
	{
		*ppSurface = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_SCRATCH, &surface, nullptr);

	if (SUCCEEDED(hr))
	{
		*ppSurface = new IDirect3DSurface8(this, surface);
	}
	else
	{
		*ppSurface = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CopyRects(IDirect3DSurface8 *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, IDirect3DSurface8 *pDestinationSurface, CONST POINT *pDestPointsArray)
{
	if (pSourceSurface == nullptr || pDestinationSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DSURFACE_DESC descSource, descDestination;
	pSourceSurface->mProxy->GetDesc(&descSource);
	pDestinationSurface->mProxy->GetDesc(&descDestination);

	HRESULT hr = D3DERR_INVALIDCALL;

	if ((descSource.Pool == D3DPOOL_SYSTEMMEM && descSource.Pool == D3DPOOL_DEFAULT) && descSource.Format == descDestination.Format)
	{
		if (pSourceRectsArray != nullptr)
		{
			for (UINT i = 0; i < cRects; ++i)
			{
				POINT pt;

				if (pDestPointsArray != nullptr)
				{
					pt.x = pDestPointsArray[i].x;
					pt.y = pDestPointsArray[i].y;
				}
				else
				{
					pt.x = pSourceRectsArray[i].left;
					pt.y = pSourceRectsArray[i].top;
				}

				hr = this->mProxy->UpdateSurface(pSourceSurface->mProxy, &pSourceRectsArray[i], pDestinationSurface->mProxy, &pt);
			
				if (FAILED(hr))
				{
					break;
				}
			}
		}
		else
		{
			hr = this->mProxy->UpdateSurface(pSourceSurface->mProxy, nullptr, pDestinationSurface->mProxy, nullptr);
		}
	}
	else if (descSource.Pool == D3DPOOL_DEFAULT && descDestination.Pool == D3DPOOL_DEFAULT)
	{
		if (pSourceRectsArray != nullptr)
		{
			for (UINT i = 0; i < cRects; ++i)
			{
				RECT rect;

				if (pDestPointsArray != nullptr)
				{
					rect.left = pDestPointsArray[i].x;
					rect.right = rect.left + (pSourceRectsArray[i].right - pSourceRectsArray[i].left);
					rect.top = pDestPointsArray[i].y;
					rect.bottom = rect.top - (pSourceRectsArray[i].top - pSourceRectsArray[i].bottom);
				}
				else
				{
					rect = pSourceRectsArray[i];
				}

				hr = this->mProxy->StretchRect(pSourceSurface->mProxy, &pSourceRectsArray[i], pDestinationSurface->mProxy, &rect, D3DTEXF_NONE);
			}
		}
		else
		{
			hr = this->mProxy->StretchRect(pSourceSurface->mProxy, nullptr, pDestinationSurface->mProxy, nullptr, D3DTEXF_NONE);
		}
	}
	else
	{
		LOG(WARNING) << "'IDirect3DDevice8::CopyRects' failed because surface properties do not allow conversion to Direct3D 9 call!";
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::UpdateTexture(IDirect3DBaseTexture8 *pSourceTexture, IDirect3DBaseTexture8 *pDestinationTexture)
{
	if (pSourceTexture == nullptr || pDestinationTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mProxy->UpdateTexture(static_cast<IDirect3DBaseTexture9 *>(pSourceTexture->mProxy), static_cast<IDirect3DBaseTexture9 *>(pDestinationTexture->mProxy));
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetFrontBuffer(IDirect3DSurface8 *pDestSurface)
{
	if (pDestSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mProxy->GetFrontBufferData(0, pDestSurface->mProxy);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetRenderTarget(IDirect3DSurface8 *pRenderTarget, IDirect3DSurface8 *pNewZStencil)
{
	IDirect3DSurface9 *rendertarget = nullptr, *depthstencil = nullptr;

	if (pRenderTarget != nullptr)
	{
		rendertarget = pRenderTarget->mProxy;
	}
	if (pNewZStencil != nullptr)
	{
		depthstencil = pNewZStencil->mProxy;
	}
		
	HRESULT hr = this->mProxy->SetRenderTarget(0, rendertarget);

	if (FAILED(hr))
	{
		return hr;
	}

	return this->mProxy->SetDepthStencilSurface(depthstencil);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetRenderTarget(IDirect3DSurface8 **ppRenderTarget)
{
	if (ppRenderTarget == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->GetRenderTarget(0, &surface);

	if (SUCCEEDED(hr) && surface != nullptr)
	{
		*ppRenderTarget = new IDirect3DSurface8(this, surface);
	}
	else
	{
		*ppRenderTarget = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetDepthStencilSurface(IDirect3DSurface8 **ppZStencilSurface)
{
	if (ppZStencilSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *surface = nullptr;

	HRESULT hr = this->mProxy->GetDepthStencilSurface(&surface);

	if (SUCCEEDED(hr) && surface != nullptr)
	{
		*ppZStencilSurface = new IDirect3DSurface8(this, surface);
	}
	else
	{
		*ppZStencilSurface = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::BeginScene(void)
{
	return this->mProxy->BeginScene();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::EndScene(void)
{
	return this->mProxy->EndScene();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return this->mProxy->Clear(Count, pRects, Flags, Color, Z, Stencil);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return this->mProxy->SetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return this->mProxy->GetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return this->mProxy->MultiplyTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetViewport(CONST D3DVIEWPORT8 *pViewport)
{
	return this->mProxy->SetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetViewport(D3DVIEWPORT8 *pViewport)
{
	return this->mProxy->GetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetMaterial(CONST D3DMATERIAL8 *pMaterial)
{
	return this->mProxy->SetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetMaterial(D3DMATERIAL8 *pMaterial)
{
	return this->mProxy->GetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetLight(DWORD Index, CONST D3DLIGHT8 *pLight)
{
	return this->mProxy->SetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetLight(DWORD Index, D3DLIGHT8 *pLight)
{
	return this->mProxy->GetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::LightEnable(DWORD Index, BOOL Enable)
{
	return this->mProxy->LightEnable(Index, Enable);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return this->mProxy->GetLightEnable(Index, pEnable);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	return this->mProxy->SetClipPlane(Index, pPlane);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetClipPlane(DWORD Index, float *pPlane)
{
	return this->mProxy->GetClipPlane(Index, pPlane);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
#define D3DRS_SOFTWAREVERTEXPROCESSING 153
#define D3DRS_PATCHSEGMENTS 164

	switch (static_cast<DWORD>(State))
	{
		case D3DRS_SOFTWAREVERTEXPROCESSING:
			return this->mProxy->SetSoftwareVertexProcessing(Value);
		case D3DRS_PATCHSEGMENTS:
			return D3DERR_INVALIDCALL;
		default:
			return this->mProxy->SetRenderState(State, Value);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	if (pValue == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	switch (static_cast<DWORD>(State))
	{
		case D3DRS_SOFTWAREVERTEXPROCESSING:
			*pValue = this->mProxy->GetSoftwareVertexProcessing();
			return D3D_OK;
		case D3DRS_PATCHSEGMENTS:
			*pValue = 0;
			return D3DERR_INVALIDCALL;
		default:
			return this->mProxy->GetRenderState(State, pValue);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::BeginStateBlock(void)
{
	return this->mProxy->BeginStateBlock();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::EndStateBlock(DWORD *pToken)
{
	if (pToken == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DStateBlock9 *stateblock = nullptr;

	HRESULT hr = this->mProxy->EndStateBlock(&stateblock);

	if (SUCCEEDED(hr))
	{
		*pToken = static_cast<DWORD>(this->mStateBlocks.size()) + 1;

		this->mStateBlocks.push_back(stateblock);
	}
	else
	{
		*pToken = 0xFFFFFFFF;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::ApplyStateBlock(DWORD Token)
{
	if (Token == 0 || Token == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Token -= 1;

	if (Token >= static_cast<DWORD>(this->mStateBlocks.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mStateBlocks[Token]->Apply();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CaptureStateBlock(DWORD Token)
{
	if (Token == 0 || Token == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Token -= 1;

	if (Token >= static_cast<DWORD>(this->mStateBlocks.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mStateBlocks[Token]->Capture();
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DeleteStateBlock(DWORD Token)
{
	if (Token == 0 || Token == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Token -= 1;

	if (Token >= static_cast<DWORD>(this->mStateBlocks.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	this->mStateBlocks[Token]->Release();
	this->mStateBlocks[Token] = nullptr;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD *pToken)
{
	LOG(INFO) << "Redirecting '" << "IDirect3DDevice8::CreateStateBlock" << "(" << Type << ", " << pToken << ")' ...";

	if (pToken == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DStateBlock9 *stateblock = nullptr;

	HRESULT hr = this->mProxy->CreateStateBlock(Type, &stateblock);

	if (SUCCEEDED(hr))
	{
		*pToken = static_cast<DWORD>(this->mStateBlocks.size()) + 1;

		this->mStateBlocks.push_back(stateblock);
	}
	else
	{
		LOG(WARNING) << "> 'IDirect3DDevice9::CreateStateBlock' failed with '" << DXGetErrorStringA(hr) << "'!";

		*pToken = 0xFFFFFFFF;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetClipStatus(CONST D3DCLIPSTATUS8 *pClipStatus)
{
	return this->mProxy->SetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetClipStatus(D3DCLIPSTATUS8 *pClipStatus)
{
	return this->mProxy->GetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetTexture(DWORD Stage, IDirect3DBaseTexture8 **ppTexture)
{
	if (ppTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DBaseTexture9 *texture = nullptr;

	HRESULT hr = this->mProxy->GetTexture(Stage, &texture);

	if (SUCCEEDED(hr) && texture != nullptr)
	{
		switch (texture->GetType())
		{
			case D3DRTYPE_TEXTURE:
				*ppTexture = new IDirect3DTexture8(this, static_cast<IDirect3DTexture9 *>(texture));
				break;
			case D3DRTYPE_VOLUMETEXTURE:
				*ppTexture = new IDirect3DVolumeTexture8(this, static_cast<IDirect3DVolumeTexture9 *>(texture));
				break;
			case D3DRTYPE_CUBETEXTURE:
				*ppTexture = new IDirect3DCubeTexture8(this, static_cast<IDirect3DCubeTexture9 *>(texture));
				break;
			default:
				assert(false);
		}
	}
	else
	{
		*ppTexture = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture)
{
	IDirect3DBaseTexture9 *texture = nullptr;

	if (pTexture != nullptr)
	{
		texture = static_cast<IDirect3DBaseTexture9 *>(pTexture->mProxy);
	}

	return this->mProxy->SetTexture(Stage, texture);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
#define D3DTSS_ADDRESSU 13
#define D3DTSS_ADDRESSV 14
#define D3DTSS_ADDRESSW 25
#define D3DTSS_BORDERCOLOR 15
#define D3DTSS_MAGFILTER 16
#define D3DTSS_MINFILTER 17
#define D3DTSS_MIPFILTER 18
#define D3DTSS_MIPMAPLODBIAS 19
#define D3DTSS_MAXMIPLEVEL 20
#define D3DTSS_MAXANISOTROPY 21

	switch (static_cast<DWORD>(Type))
	{
		case D3DTSS_ADDRESSU:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_ADDRESSU, pValue);
		case D3DTSS_ADDRESSV:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_ADDRESSV, pValue);
		case D3DTSS_ADDRESSW:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_ADDRESSW, pValue);
		case D3DTSS_BORDERCOLOR:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_BORDERCOLOR, pValue);
		case D3DTSS_MAGFILTER:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_MAGFILTER, pValue);
		case D3DTSS_MINFILTER:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_MINFILTER, pValue);
		case D3DTSS_MIPFILTER:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_MIPFILTER, pValue);
		case D3DTSS_MIPMAPLODBIAS:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, pValue);
		case D3DTSS_MAXMIPLEVEL:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, pValue);
		case D3DTSS_MAXANISOTROPY:
			return this->mProxy->GetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, pValue);
		default:
			return this->mProxy->GetTextureStageState(Stage, Type, pValue);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	switch (static_cast<DWORD>(Type))
	{
		case D3DTSS_ADDRESSU:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_ADDRESSU, Value);
		case D3DTSS_ADDRESSV:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_ADDRESSV, Value);
		case D3DTSS_ADDRESSW:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_ADDRESSW, Value);
		case D3DTSS_BORDERCOLOR:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_BORDERCOLOR, Value);
		case D3DTSS_MAGFILTER:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_MAGFILTER, Value);
		case D3DTSS_MINFILTER:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_MINFILTER, Value);
		case D3DTSS_MIPFILTER:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_MIPFILTER, Value);
		case D3DTSS_MIPMAPLODBIAS:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, Value);
		case D3DTSS_MAXMIPLEVEL:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, Value);
		case D3DTSS_MAXANISOTROPY:
			return this->mProxy->SetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, Value);
		default:
			return this->mProxy->SetTextureStageState(Stage, Type, Value);
	}
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::ValidateDevice(DWORD *pNumPasses)
{
	return this->mProxy->ValidateDevice(pNumPasses);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetInfo(DWORD DevInfoID, void *pDevInfoStruct, DWORD DevInfoStructSize)
{
	UNREFERENCED_PARAMETER(DevInfoID);
	UNREFERENCED_PARAMETER(pDevInfoStruct);
	UNREFERENCED_PARAMETER(DevInfoStructSize);

	LOG(WARNING) << "Redirecting '" << "IDirect3DDevice8::GetInfo" << "(" << this << ", " << DevInfoID << ", " << pDevInfoStruct << ", " << DevInfoStructSize << ")' ...";

	return S_FALSE;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	return this->mProxy->SetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return this->mProxy->GetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return this->mProxy->SetCurrentTexturePalette(PaletteNumber);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	return this->mProxy->GetCurrentTexturePalette(pPaletteNumber);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return this->mProxy->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	return this->mProxy->DrawIndexedPrimitive(PrimitiveType, this->mBaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return this->mProxy->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return this->mProxy->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8 *pDestBuffer, DWORD Flags)
{
	if (pDestBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DVertexBuffer9 *buffer = static_cast<IDirect3DVertexBuffer9 *>(pDestBuffer->mProxy);
	IDirect3DVertexDeclaration9 *declaration = nullptr;

	HRESULT hr = this->mProxy->GetVertexDeclaration(&declaration);

	if (FAILED(hr))
	{
		return hr;
	}

	hr = this->mProxy->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, buffer, declaration, Flags);

	declaration->Release();

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreateVertexShader(CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage)
{
	UNREFERENCED_PARAMETER(Usage);

	LOG(INFO) << "Redirecting '" << "IDirect3DDevice8::CreateVertexShader" << "(" << this << ", " << pDeclaration << ", " << pFunction << ", " << pHandle << ", " << Usage << ")' ...";

	if (pDeclaration == nullptr || pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	UINT i = 0;
	std::size_t tokens = 0;
	WORD stream = 0, offset = 0;
	D3DVERTEXELEMENT9 elements[32];

#define D3DVSD_CONSTADDRESSSHIFT 0
#define D3DVSD_CONSTADDRESSMASK (0x7F << D3DVSD_CONSTADDRESSSHIFT)
#define D3DVSD_EXTINFOSHIFT 0
#define D3DVSD_EXTINFOMASK (0xFFFFFF << D3DVSD_EXTINFOSHIFT)
#define D3DVSD_STREAMNUMBERSHIFT 0
#define D3DVSD_STREAMNUMBERMASK (0xF << D3DVSD_STREAMNUMBERSHIFT)
#define D3DVSD_VERTEXREGSHIFT 0
#define D3DVSD_VERTEXREGMASK (0x1F << D3DVSD_VERTEXREGSHIFT)
#define D3DVSD_CONSTRSSHIFT 16
#define D3DVSD_CONSTRSMASK (0x1FFF << D3DVSD_CONSTRSSHIFT)
#define D3DVSD_DATATYPESHIFT 16
#define D3DVSD_DATATYPEMASK (0xF << D3DVSD_DATATYPESHIFT)
#define D3DVSD_SKIPCOUNTSHIFT 16
#define D3DVSD_SKIPCOUNTMASK (0xF << D3DVSD_SKIPCOUNTSHIFT)
#define D3DVSD_VERTEXREGINSHIFT 20
#define D3DVSD_VERTEXREGINMASK (0xF << D3DVSD_VERTEXREGINSHIFT)
#define D3DVSD_EXTCOUNTSHIFT 24
#define D3DVSD_EXTCOUNTMASK (0x1F << D3DVSD_EXTCOUNTSHIFT)
#define D3DVSD_CONSTCOUNTSHIFT 25
#define D3DVSD_CONSTCOUNTMASK (0xF << D3DVSD_CONSTCOUNTSHIFT)
#define D3DVSD_DATALOADTYPESHIFT 28
#define D3DVSD_DATALOADTYPEMASK (0x1 << D3DVSD_DATALOADTYPESHIFT)
#define D3DVSD_STREAMTESSSHIFT 28
#define D3DVSD_STREAMTESSMASK (0x1 << D3DVSD_STREAMTESSSHIFT)
#define D3DVSD_TOKENTYPESHIFT 29
#define D3DVSD_TOKENTYPEMASK (0x7 << D3DVSD_TOKENTYPESHIFT)
#define D3DVSD_TOKEN_STREAM 1
#define D3DVSD_TOKEN_STREAMDATA 2
#define D3DVSD_TOKEN_TESSELLATOR 3
#define D3DVSD_TOKEN_CONSTMEM 4
#define D3DVSD_TOKEN_EXT 5

	LOG(INFO) << "> Translating vertex declaration ...";

	const BYTE sTypes[][2] =
	{
		{ D3DDECLTYPE_FLOAT1, 4 },
		{ D3DDECLTYPE_FLOAT2, 8 },
		{ D3DDECLTYPE_FLOAT3, 12 },
		{ D3DDECLTYPE_FLOAT4, 16 },
		{ D3DDECLTYPE_D3DCOLOR, 4 },
		{ D3DDECLTYPE_UBYTE4, 4 },
		{ D3DDECLTYPE_SHORT2, 4 },
		{ D3DDECLTYPE_SHORT4, 8 },
		{ D3DDECLTYPE_UBYTE4N, 4 },
		{ D3DDECLTYPE_SHORT2N, 4 },
		{ D3DDECLTYPE_SHORT4N, 8 },
		{ D3DDECLTYPE_USHORT2N, 4 },
		{ D3DDECLTYPE_USHORT4N, 8 },
		{ D3DDECLTYPE_UDEC3, 6 },
		{ D3DDECLTYPE_DEC3N, 6 },
		{ D3DDECLTYPE_FLOAT16_2, 8 },
		{ D3DDECLTYPE_FLOAT16_4, 16 }
	};
	const BYTE sAddressUsage[][2] =
	{
		{ D3DDECLUSAGE_POSITION, 0 },
		{ D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ D3DDECLUSAGE_BLENDINDICES, 0 },
		{ D3DDECLUSAGE_NORMAL, 0 },
		{ D3DDECLUSAGE_PSIZE, 0 },
		{ D3DDECLUSAGE_COLOR, 0 },
		{ D3DDECLUSAGE_COLOR, 1 },
		{ D3DDECLUSAGE_TEXCOORD, 0 },
		{ D3DDECLUSAGE_TEXCOORD, 1 },
		{ D3DDECLUSAGE_TEXCOORD, 2 },
		{ D3DDECLUSAGE_TEXCOORD, 3 },
		{ D3DDECLUSAGE_TEXCOORD, 4 },
		{ D3DDECLUSAGE_TEXCOORD, 5 },
		{ D3DDECLUSAGE_TEXCOORD, 6 },
		{ D3DDECLUSAGE_TEXCOORD, 7 },
		{ D3DDECLUSAGE_POSITION, 1 },
		{ D3DDECLUSAGE_NORMAL, 1 }
	};

	while (i < (ARRAYSIZE(elements) - 1))
	{
		const DWORD token = *pDeclaration;
		const DWORD tokenType = (token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;

#define D3DVSD_END() 0xFFFFFFFF

		if (token == D3DVSD_END())
		{
			break;
		}
		else if (tokenType == D3DVSD_TOKEN_STREAM)
		{
			stream = static_cast<WORD>((token & D3DVSD_STREAMNUMBERMASK) >> D3DVSD_STREAMNUMBERSHIFT);
			offset = 0;
		}
		else if (tokenType == D3DVSD_TOKEN_STREAMDATA && !(token & 0x10000000))
		{		
			elements[i].Stream = stream;
			elements[i].Offset = offset;
			const DWORD type = (token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT;
			elements[i].Type = sTypes[type][0];
			offset += sTypes[type][1];
			elements[i].Method = D3DDECLMETHOD_DEFAULT;
			const DWORD address = (token & D3DVSD_VERTEXREGMASK) >> D3DVSD_VERTEXREGSHIFT;
			elements[i].Usage = sAddressUsage[address][0];
			elements[i].UsageIndex = sAddressUsage[address][1];

			++i;
		}
		else if (tokenType == D3DVSD_TOKEN_STREAMDATA && (token & 0x10000000))
		{
			offset += ((token & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT) * sizeof(DWORD);
		}
		else if (tokenType == D3DVSD_TOKEN_TESSELLATOR && !(token & 0x10000000))
		{
			elements[i].Stream = stream;
			elements[i].Offset = offset;

			const DWORD input = (token & D3DVSD_VERTEXREGINMASK) >> D3DVSD_VERTEXREGINSHIFT;

			for (UINT r = 0; r < i; ++r)
			{
				if (elements[r].Usage == sAddressUsage[input][0] && elements[r].UsageIndex == sAddressUsage[input][1])
				{
					elements[i].Stream = elements[r].Stream;
					elements[i].Offset = elements[r].Offset;
					break;
				}
			}

			elements[i].Type = D3DDECLTYPE_FLOAT3;
			elements[i].Method = D3DDECLMETHOD_CROSSUV;
			const DWORD address = (token & 0xF);
			elements[i].Usage = sAddressUsage[address][0];
			elements[i].UsageIndex = sAddressUsage[address][1];

			++i;
		}
		else if (tokenType == D3DVSD_TOKEN_TESSELLATOR && (token & 0x10000000))
		{
			elements[i].Stream = 0;
			elements[i].Offset = 0;
			elements[i].Type = D3DDECLTYPE_UNUSED;
			elements[i].Method = D3DDECLMETHOD_UV;
			const DWORD address = (token & 0xF);
			elements[i].Usage = sAddressUsage[address][0];
			elements[i].UsageIndex = sAddressUsage[address][1];

			++i;
		}
		else if (tokenType == D3DVSD_TOKEN_CONSTMEM)
		{
			LOG(WARNING) << "> Failed because 'D3DVSD_TOKEN_CONSTMEM' is not supported!";

			return D3DERR_NOTAVAILABLE;
		}
		
		++tokens;
		++pDeclaration;
	}

	LOG(TRACE) << "  +------------+------------+--------------+--------------+--------------+------------+";
	LOG(TRACE) << "  | Stream     | Offset     | Type         | Method       | Usage        | UsageIndex |";
	LOG(TRACE) << "  +------------+------------+--------------+--------------+--------------+------------+";

	for (UINT k = 0; k < i; ++k)
	{
		char line[88];
		sprintf_s(line, "  | %-10hu | %-10hu | 0x%010hhX | 0x%010hhX | 0x%010hhX | %-10hhu |", elements[k].Stream, elements[k].Offset, elements[k].Type, elements[k].Method, elements[k].Usage, elements[k].UsageIndex);

		LOG(TRACE) << line;
	}

	LOG(TRACE) << "  +------------+------------+--------------+--------------+--------------+------------+";

	const D3DVERTEXELEMENT9 terminator = D3DDECL_END();
	elements[i] = terminator;

	LOG(INFO) << "> Disassembling shader and translating assembly to Direct3D 9 compatible code ...";

	ID3DXBuffer *disassembly, *assembly;
	IDirect3DVertexShader9 *shader;
	IDirect3DVertexDeclaration9 *declaration;

	HRESULT hr = ::D3DXDisassembleShader(pFunction, FALSE, nullptr, &disassembly);

	if (FAILED(hr))
	{
		LOG(ERROR) << "'D3DXDisassembleShader' failed with '" << DXGetErrorStringA(hr) << "'!";

		return hr;
	}

	std::string source(static_cast<LPCSTR>(disassembly->GetBufferPointer()), disassembly->GetBufferSize());
	std::size_t declpos = source.find("vs_1_1") + 7;

	for (UINT k = 0; k < i; ++k)
	{
		std::string decl;

		switch (elements[k].Usage)
		{
			case D3DDECLUSAGE_POSITION:
				decl = "dcl_position";
				break;
			case D3DDECLUSAGE_BLENDWEIGHT:
				decl = "dcl_blendweight";
				break;
			case D3DDECLUSAGE_BLENDINDICES:
				decl = "dcl_blendindices";
				break;
			case D3DDECLUSAGE_NORMAL:
				decl = "dcl_normal";
				break;
			case D3DDECLUSAGE_PSIZE:
				decl = "dcl_psize";
				break;
			case D3DDECLUSAGE_COLOR:
				decl = "dcl_color";
				break;
			case D3DDECLUSAGE_TEXCOORD:
				decl = "dcl_texcoord";
				break;
		}

		if (elements[i].UsageIndex > 0)
		{
			decl += std::to_string(elements[i].UsageIndex);
		}

		decl += " v" + std::to_string(k) + "\n";

		source.insert(declpos, decl);
		declpos += decl.length();
	}

	hr = ::D3DXAssembleShader(source.data(), static_cast<UINT>(source.size()), nullptr, nullptr, D3DXSHADER_SKIPVALIDATION, &assembly, nullptr);

	disassembly->Release();

	if (FAILED(hr))
	{
		LOG(ERROR) << "'D3DXAssembleShader' failed with '" << DXGetErrorStringA(hr) << "'!";

		return hr;
	}

	hr = this->mProxy->CreateVertexShader(static_cast<const DWORD *>(assembly->GetBufferPointer()), &shader);

	assembly->Release();

	if (SUCCEEDED(hr))
	{
		hr = this->mProxy->CreateVertexDeclaration(elements, &declaration);

		if (SUCCEEDED(hr))
		{
			*pHandle = static_cast<DWORD>(this->mVertexShaders.size()) + 0xF0000000;

			this->mVertexShaders.push_back(std::make_pair(shader, declaration));

			DWORD *pDeclarationClone = new DWORD[tokens];
			std::memcpy(pDeclarationClone, pDeclaration, tokens * sizeof(DWORD));

			this->mVertexShaderDeclarations.push_back(std::make_pair(pDeclarationClone, tokens * sizeof(DWORD)));
		}
		else
		{
			LOG(ERROR) << "'IDirect3DDevice9::CreateVertexDeclaration' failed with '" << DXGetErrorStringA(hr) << "'!";

			shader->Release();
		}
	}
	else
	{
		LOG(ERROR) << "'IDirect3DDevice9::CreateVertexShader' failed with '" << DXGetErrorStringA(hr) << "'!";
	}
		
	if (FAILED(hr))
	{
		*pHandle = 0xFFFFFFFF;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetVertexShader(DWORD Handle)
{
	if (Handle == 0)
	{
		this->mProxy->SetFVF(0);
		this->mProxy->SetVertexDeclaration(nullptr);

		return this->mProxy->SetVertexShader(nullptr);
	}
	else if (Handle == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}
	else if (Handle < 0xF0000000)
	{
		this->mProxy->SetVertexShader(nullptr);

		return this->mProxy->SetFVF(Handle);
	}

	Handle -= 0xF0000000;

	if (Handle >= static_cast<DWORD>(this->mVertexShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = this->mProxy->SetVertexShader(this->mVertexShaders[Handle].first);

	if (FAILED(hr))
	{
		return hr;
	}

	return this->mProxy->SetVertexDeclaration(this->mVertexShaders[Handle].second);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetVertexShader(DWORD *pHandle)
{
	if (pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DVertexShader9 *shader = nullptr;

	HRESULT hr = this->mProxy->GetVertexShader(&shader);

	if (SUCCEEDED(hr) && shader != nullptr)
	{
		*pHandle = 0xFFFFFFFF;

		for (std::size_t i = 0, count = this->mVertexShaders.size(); i < count; ++i)
		{
			if (this->mVertexShaders[i].first == shader)
			{
				*pHandle = static_cast<DWORD>(i) + 0xF0000000;
				break;
			}
		}
	}
	else
	{
		*pHandle = 0;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DeleteVertexShader(DWORD Handle)
{
	if (Handle == 0 || Handle == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Handle -= 0xF0000000;

	if (Handle >= static_cast<DWORD>(this->mVertexShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	this->mVertexShaders[Handle].first->Release();
	this->mVertexShaders[Handle].first = nullptr;
	this->mVertexShaders[Handle].second->Release();
	this->mVertexShaders[Handle].second = nullptr;
	delete[] this->mVertexShaderDeclarations[Handle].first;
	this->mVertexShaderDeclarations[Handle].first = nullptr;
	this->mVertexShaderDeclarations[Handle].second = 0;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetVertexShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount)
{
	return this->mProxy->SetVertexShaderConstantF(Register, static_cast<CONST float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetVertexShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount)
{
	return this->mProxy->GetVertexShaderConstantF(Register, static_cast<float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
	if (Handle == 0 || Handle == 0xFFFFFFFF || Handle < 0xF0000000)
	{
		return D3DERR_INVALIDCALL;
	}

	Handle -= 0xF0000000;

	if (Handle >= static_cast<DWORD>(this->mVertexShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	std::memcpy(pData, this->mVertexShaderDeclarations[Handle].first, *pSizeOfData = std::min(*pSizeOfData, static_cast<DWORD>(this->mVertexShaderDeclarations[Handle].second)));

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetVertexShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
	if (Handle == 0 || Handle == 0xFFFFFFFF || Handle <= 0xF0000000)
	{
		return D3DERR_INVALIDCALL;
	}

	Handle -= 0xF0000000;

	if (Handle >= static_cast<DWORD>(this->mVertexShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mVertexShaders[Handle].first->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 *pStreamData, UINT Stride)
{
	IDirect3DVertexBuffer9 *buffer = nullptr;

	if (pStreamData != nullptr)
	{
		buffer = static_cast<IDirect3DVertexBuffer9 *>(pStreamData->mProxy);
	}

	return this->mProxy->SetStreamSource(StreamNumber, buffer, 0, Stride);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 **ppStreamData, UINT *pStride)
{
	if (ppStreamData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	UINT offset;
	IDirect3DVertexBuffer9 *buffer = nullptr;

	HRESULT hr = this->mProxy->GetStreamSource(StreamNumber, &buffer, &offset, pStride);

	if (SUCCEEDED(hr) && buffer != nullptr)
	{
		*ppStreamData = new IDirect3DVertexBuffer8(this, buffer);
	}
	else
	{
		*ppStreamData = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetIndices(IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex)
{
	IDirect3DIndexBuffer9 *buffer = nullptr;

	if (pIndexData != nullptr)
	{
		buffer = static_cast<IDirect3DIndexBuffer9 *>(pIndexData->mProxy);
	}

	HRESULT hr = this->mProxy->SetIndices(buffer);

	if (SUCCEEDED(hr))
	{
		this->mBaseVertexIndex = static_cast<INT>(BaseVertexIndex);
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetIndices(IDirect3DIndexBuffer8 **ppIndexData, UINT *pBaseVertexIndex)
{
	if (ppIndexData == nullptr || pBaseVertexIndex == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}
		
	*pBaseVertexIndex = static_cast<UINT>(this->mBaseVertexIndex);

	IDirect3DIndexBuffer9 *buffer = nullptr;

	HRESULT hr = this->mProxy->GetIndices(&buffer);

	if (SUCCEEDED(hr) && buffer != nullptr)
	{
		*ppIndexData = new IDirect3DIndexBuffer8(this, buffer);
	}
	else
	{
		*ppIndexData = nullptr;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::CreatePixelShader(CONST DWORD *pFunction, DWORD *pHandle)
{
	LOG(INFO) << "Redirecting '" << "IDirect3DDevice8::CreatePixelShader" << "(" << this << ", " << pFunction << ", " << pHandle << ")' ...";

	if (pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DPixelShader9 *shader = nullptr;

	HRESULT hr = this->mProxy->CreatePixelShader(pFunction, &shader);

	if (SUCCEEDED(hr))
	{
		*pHandle = static_cast<DWORD>(this->mPixelShaders.size()) + 0xF0000000;

		this->mPixelShaders.push_back(shader);
	}
	else
	{
		LOG(ERROR) << "'IDirect3DDevice9::CreatePixelShader' failed with '" << DXGetErrorStringA(hr) << "'!";

		*pHandle = 0xFFFFFFFF;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetPixelShader(DWORD Handle)
{
	if (Handle == 0)
	{
		return this->mProxy->SetPixelShader(nullptr);
	}
	else if (Handle == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Handle -= 0xF0000000;

	if (Handle >= static_cast<DWORD>(this->mPixelShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mProxy->SetPixelShader(this->mPixelShaders[Handle]);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetPixelShader(DWORD *pHandle)
{
	if (pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DPixelShader9 *shader = nullptr;

	HRESULT hr = this->mProxy->GetPixelShader(&shader);

	if (SUCCEEDED(hr) && shader != nullptr)
	{
		*pHandle = 0xFFFFFFFF;

		for (std::size_t i = 0, count = this->mPixelShaders.size(); i < count; ++i)
		{
			if (this->mPixelShaders[i] == shader)
			{
				*pHandle = static_cast<DWORD>(i) + 0xF0000000;
				break;
			}
		}
	}
	else
	{
		*pHandle = 0;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DeletePixelShader(DWORD Handle)
{
	if (Handle == 0 || Handle == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Handle -= 0xF0000000;
		
	if (Handle >= static_cast<DWORD>(this->mPixelShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	this->mPixelShaders[Handle]->Release();
	this->mPixelShaders[Handle] = nullptr;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::SetPixelShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount)
{
	return this->mProxy->SetPixelShaderConstantF(Register, static_cast<CONST float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount)
{
	return this->mProxy->GetPixelShaderConstantF(Register, static_cast<float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::GetPixelShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
	if (Handle == 0 || Handle == 0xFFFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	Handle -= 0xF0000000;

	if (Handle >= static_cast<DWORD>(this->mPixelShaders.size()))
	{
		return D3DERR_INVALIDCALL;
	}

	return this->mPixelShaders[Handle]->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return this->mProxy->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return this->mProxy->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}
HRESULT STDMETHODCALLTYPE										IDirect3DDevice8::DeletePatch(UINT Handle)
{
	return this->mProxy->DeletePatch(Handle);
}

// IDirect3D8
HRESULT STDMETHODCALLTYPE										IDirect3D8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}
	else if (riid == IID_IUnknown || riid == IID_IDirect3D8)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}
	else
	{
		return this->mProxy->QueryInterface(riid, ppvObj);
	}
}
ULONG STDMETHODCALLTYPE											IDirect3D8::AddRef(void)
{
	return this->mProxy->AddRef();
}
ULONG STDMETHODCALLTYPE											IDirect3D8::Release(void)
{
	const ULONG ref = this->mProxy->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return this->mProxy->RegisterSoftwareDevice(pInitializeFunction);
}
UINT STDMETHODCALLTYPE											IDirect3D8::GetAdapterCount(void)
{
	return this->mProxy->GetAdapterCount();
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier)
{
	D3DADAPTER_IDENTIFIER9 identifier;

#define D3DENUM_NO_WHQL_LEVEL 0x00000002L

	if ((Flags & D3DENUM_NO_WHQL_LEVEL) == 0)
	{
		Flags |= D3DENUM_WHQL_LEVEL;
	}

	HRESULT hr = this->mProxy->GetAdapterIdentifier(Adapter, Flags, &identifier);

	if (SUCCEEDED(hr))
	{
		std::memcpy(pIdentifier->Driver, identifier.Driver, MAX_DEVICE_IDENTIFIER_STRING * sizeof(char));
		std::memcpy(pIdentifier->Description, identifier.Description, MAX_DEVICE_IDENTIFIER_STRING * sizeof(char));
		pIdentifier->DriverVersion = identifier.DriverVersion;
		pIdentifier->VendorId = identifier.VendorId;
		pIdentifier->DeviceId = identifier.DeviceId;
		pIdentifier->SubSysId = identifier.SubSysId;
		pIdentifier->Revision = identifier.Revision;
		pIdentifier->DeviceIdentifier = identifier.DeviceIdentifier;
		pIdentifier->WHQLLevel = identifier.WHQLLevel;
	}

	return hr;
}
UINT STDMETHODCALLTYPE											IDirect3D8::GetAdapterModeCount(UINT Adapter)
{
	return this->mProxy->GetAdapterModeCount(Adapter, D3DFMT_X8R8G8B8);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode)
{
	return this->mProxy->EnumAdapterModes(Adapter, D3DFMT_X8R8G8B8, Mode, pMode);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return this->mProxy->GetAdapterDisplayMode(Adapter, pMode);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return this->mProxy->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, bWindowed);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return this->mProxy->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{
	return this->mProxy->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, nullptr);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return this->mProxy->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps)
{
	if (pCaps == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DCAPS9 caps;

	HRESULT hr = this->mProxy->GetDeviceCaps(Adapter, DeviceType, &caps);

	if (SUCCEEDED(hr))
	{
		std::memcpy(pCaps, &caps, sizeof(D3DCAPS8));
	}

	return hr;
}
HMONITOR STDMETHODCALLTYPE										IDirect3D8::GetAdapterMonitor(UINT Adapter)
{
	return this->mProxy->GetAdapterMonitor(Adapter);
}
HRESULT STDMETHODCALLTYPE										IDirect3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS8 *pPresentationParameters, IDirect3DDevice8 **ppReturnedDeviceInterface)
{
	LOG(INFO) << "Redirecting '" << "IDirect3D8::CreateDevice" << "(" << this << ", " << Adapter << ", " << DeviceType << ", " << hFocusWindow << ", " << BehaviorFlags << ", " << pPresentationParameters << ", " << ppReturnedDeviceInterface << ")' ...";

	if (pPresentationParameters == nullptr || ppReturnedDeviceInterface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DPRESENT_PARAMETERS pp;
	pp.BackBufferWidth = pPresentationParameters->BackBufferWidth;
	pp.BackBufferHeight = pPresentationParameters->BackBufferHeight;
	pp.BackBufferFormat = pPresentationParameters->BackBufferFormat;
	pp.BackBufferCount = pPresentationParameters->BackBufferCount;
	pp.MultiSampleType = pPresentationParameters->MultiSampleType;
	pp.MultiSampleQuality = 0;
	pp.SwapEffect = pPresentationParameters->SwapEffect;
	pp.hDeviceWindow = pPresentationParameters->hDeviceWindow;
	pp.Windowed = pPresentationParameters->Windowed;
	pp.EnableAutoDepthStencil = pPresentationParameters->EnableAutoDepthStencil;
	pp.AutoDepthStencilFormat = pPresentationParameters->AutoDepthStencilFormat;
	pp.Flags = pPresentationParameters->Flags;
	pp.FullScreen_RefreshRateInHz = pPresentationParameters->FullScreen_RefreshRateInHz;
	pp.PresentationInterval = pPresentationParameters->FullScreen_PresentationInterval;

	if (pp.SwapEffect == D3DSWAPEFFECT_COPY_VSYNC)
	{
		LOG(WARNING) << "> 'IDirect3D8::CreateDevice' failed because 'D3DSWAPEFFECT_COPY_VSYNC' is not supported!";

		*ppReturnedDeviceInterface = nullptr;

		return D3DERR_NOTAVAILABLE;
	}
	if (pp.PresentationInterval == D3DPRESENT_RATE_UNLIMITED)
	{
		pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	IDirect3DDevice9 *device = nullptr;

	HRESULT hr = this->mProxy->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &pp, &device);

	if (SUCCEEDED(hr))
	{
		*ppReturnedDeviceInterface = new IDirect3DDevice8(this, device);
	}
	else
	{
		*ppReturnedDeviceInterface = nullptr;
	}

	return hr;
}

// D3D8
EXPORT IDirect3D8 *WINAPI										Direct3DCreate8(UINT SDKVersion)
{
	LOG(INFO) << "Redirecting '" << "Direct3DCreate8" << "(" << SDKVersion << ")' ...";
	LOG(INFO) << "> Passing on to 'Direct3DCreate9':";

	const HMODULE module = ::LoadLibraryA("d3d9.dll");

	if (module == nullptr)
	{
		LOG(ERROR) << "Failed to load Direct3D 9 module!";

		return nullptr;
	}
	
	IDirect3D9 *res = Direct3DCreate9(D3D_SDK_VERSION);

	if (res != nullptr)
	{
		return new IDirect3D8(module, res);
	}
	else
	{
		return nullptr;
	}
}