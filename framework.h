#pragma once

#include "targetver.h"
#include "resource.h"

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#define _USE_MATH_DEFINES

#include <atlbase.h>
#include <combaseapi.h>
#include <comdef.h>
#include <CommCtrl.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <D3d12SDKLayers.h>
#include <d3d12shader.h>
#include <d3d12video.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dwrite.h>
#include <dxcapi.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <evr.h>
#include <ExDisp.h>
#include <iphlpapi.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <Mmdeviceapi.h>
#include <mmsystem.h>
#include <process.h>
#include <Shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <tchar.h>
#include <time.h>
#include <wincodecsdk.h>
#include <windows.h>
#include <windowsx.h>
#include <wininet.h>
#include <winsock2.h>
#include <wrl.h>
#include <ws2tcpip.h>
#include <X3DAudio.h>
#include <xapofx.h>
#include <XAudio2.h>
#include <XAudio2fx.h>

using namespace DirectX;
using namespace Microsoft::WRL;

#include <Functiondiscoverykeys_devpkey.h>

#include <wil/com.h>
#include "WebView2.h"

#pragma comment(lib, "msxml6.lib")

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxcompiler.lib")

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")

#pragma comment(lib, "XAudio2.lib")

#pragma comment(lib, "zlibd.lib")

#pragma comment(lib, "windowscodecs.lib")

#define SAFE_DELETE(p) { delete p; p = nullptr; }
#define SAFE_DELETE_ARRAY(p) { delete[] p; p = nullptr; }
#define SAFE_RELEASE(p) { if (p) { p->Release(); p = nullptr;} }
