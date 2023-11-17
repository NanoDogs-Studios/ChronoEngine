#pragma region Includes
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#define IMGUI_DEFINE_MATH_OPERATORS
#define IDM_MYMENURESOURCE   3
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <windows.h>
#include <WinUser.h>
#include <shellapi.h>
#include <shobjidl.h> 
#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>
#include "Console.h"


#include <DirectXMath.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <assert.h>
#include <stdint.h>
#include <iostream>
#include <VersionHelpers.h>  // Include for IsWindowsVersionOrGreater function
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
using namespace DirectX;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "3DMaths.h"
#include "ObjLoading.h"
#include <windowsx.h>
#include "Hierachy.h"
#include "resource.h"
#pragma endregion

#pragma region Variables

ID3D11RasterizerState* rStateWireframe;
ID3D11RasterizerState* rStateSolid;

static bool global_windowDidResize = false;

// Function to create a popup window for project settings
bool showProjectSettingsWindow = false;

// Function to create a popup window for project settings
bool showEditorSettingsWindow = false;

string items[] = { "" };
static int current_item = 0;

bool RobotoFont = false;
bool SystemFont = true;

float fontSize = 15;

// Create an instance of the ConsoleWindow
Console console;

// Variables used for Project Assets
WCHAR selectedFilePath[MAX_PATH] = L"";


IDirectInputDevice8* DIKeyboard; // get directx keyboard
IDirectInputDevice8* DIMouse; // get directx mouse

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput; // get directx input

float rotx = 0;
float rotz = 0;
float scaleX = 1.0f;
float scaleY = 1.0f;

XMMATRIX Rotationx;
XMMATRIX Rotationz;

// Input
enum GameAction {
    GameActionMoveCamFwd,
    GameActionMoveCamBack,
    GameActionMoveCamLeft,
    GameActionMoveCamRight,
    GameActionTurnCamLeft,
    GameActionTurnCamRight,
    GameActionLookUp,
    GameActionLookDown,
    GameActionRaiseCam,
    GameActionLowerCam,
    GameActionCount
};
static bool global_keyIsDown[GameActionCount] = {};
#pragma endregion

// Function to get the installation directory
std::wstring GetInstallationDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

// Function to get the path for the project assets folder
std::wstring GetProjectAssetsFolder() {
    return GetInstallationDirectory() + L"\\ProjectAssets";
}

// Function to copy a file to the project assets folder
void CopyFileToProjectAssets(const std::wstring& sourcePath) {
    std::wstring destinationPath = GetProjectAssetsFolder() + L"\\" + sourcePath.substr(sourcePath.find_last_of(L"\\") + 1);

    if (CopyFile(sourcePath.c_str(), destinationPath.c_str(), FALSE)) {
        // Successfully copied the file
    }
    else {
        if (MessageBoxA(0, "Uhoh! it seems either a file already exists or other similar occurrences. ERR_COPYTOPROJ", "Unable to Add to project.", MB_OK) == IDOK)
        {
            return;
        }
    }
}

// Function to delete a file from the project assets folder
void DeleteFileFromProjectAssets(const std::wstring& fileName) {
    std::wstring filePath = GetProjectAssetsFolder() + L"\\" + fileName;

    if (DeleteFile(filePath.c_str())) {
        // Successfully deleted the file
    }
    else {
        // Handle error
    }
}

#pragma region Project Settings Window
// Project Settings
void RenderProjectSettingsWindow()
{
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver); // Set the Project Settings Size
    ImGui::Begin("Project Settings", &showProjectSettingsWindow);

#pragma region Project Details
    ImGui::Separator();
    ImGui::Text("Project Details");
    static char company[64] = "Something Studios";
    ImGui::InputText("Company Name", company, IM_ARRAYSIZE(company));
    static char projname[64] = "My Chrono Engine Project";
    ImGui::InputText("Project Name", projname, IM_ARRAYSIZE(projname));
#pragma endregion


#pragma region Graphics
    ImGui::Separator();
    ImGui::Text("Graphics");
    // Create a combo box to select the rendering engine
    static int selectedEngine = 0;
    const char* engines[] = { "DirectX 11", "DirectX 12", "OpenGL ES 3" };
    // Add your project settings controls here
    ImGui::Combo("Rendering Engine", &selectedEngine, engines, IM_ARRAYSIZE(engines));
    if (selectedEngine == 0) // DirectX 11
    {

    }
    if (selectedEngine == 1) // DirectX 12
    {

        if (MessageBoxA(0, "This will restart Chrono \n Are you sure you want to switch Renderers ?", "Switching Rendering Engine", MB_YESNO) == IDYES)
        {
            exit(0);
        }
        if (MessageBoxA(0, "This will restart Chrono \n Are you sure you want to switch Renderers ?", "Switching Rendering Engine", MB_YESNO) == IDNO)
        {
            return;
        }
    }
    if (selectedEngine == 2) // OpenGL ES 3
    {
        if (MessageBoxA(0, "This will restart Chrono \n Are you sure you want to switch Renderers ?", "Switching Rendering Engine", MB_YESNO) == IDYES)
        {
            exit(0);
        }
        if (MessageBoxA(0, "This will restart Chrono \n Are you sure you want to switch Renderers ?", "Switching Rendering Engine", MB_YESNO) == IDNO)
        {
            return;
        }
    }
#pragma endregion

   

    ImGui::End();
}
#pragma endregion

#pragma region Editor Settings
// Editor Settings
void RenderEditorSettingsWindow()
{
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Editor Settings", &showEditorSettingsWindow);

    ImGui::Text("Font Settings");
    ImGui::Separator();
   // ImGui::Text("Roboto Active?: %s", RobotoFont);
   // ImGui::Text("System Active?: %s", SystemFont);

    ImGui::Checkbox("Roboto", &RobotoFont);
    ImGui::Checkbox("System", &SystemFont);

    ImGui::DragFloat("Font Size", &fontSize);

    ImGui::End();
}
#pragma endregion



// Creates the render target (your gpu) object
bool win32CreateD3D11RenderTargets(ID3D11Device1* d3d11Device, IDXGISwapChain1* swapChain, ID3D11RenderTargetView** d3d11FrameBufferView, ID3D11DepthStencilView** depthBufferView)
{
    // Creates the Frame Buffer
    ID3D11Texture2D* d3d11FrameBuffer;
    HRESULT hResult = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
    assert(SUCCEEDED(hResult));

    hResult = d3d11Device->CreateRenderTargetView(d3d11FrameBuffer, 0, d3d11FrameBufferView);
    assert(SUCCEEDED(hResult));
    
    ID3D11Buffer* sphereIndexBuffer;
    ID3D11Buffer* sphereVertBuffer;

    ID3D11VertexShader* SKYMAP_VS;
    ID3D11PixelShader* SKYMAP_PS;
    ID3D10Blob* SKYMAP_VS_Buffer;
    ID3D10Blob* SKYMAP_PS_Buffer;

    ID3D11ShaderResourceView* smrv;

    ID3D11DepthStencilState* DSLessEqual;
    ID3D11RasterizerState* RSCullNone;

    int NumSphereVertices;
    int NumSphereFaces;

    XMMATRIX sphereWorld;

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    d3d11FrameBuffer->GetDesc(&depthBufferDesc);

    // release the frame buffer for rendering
    d3d11FrameBuffer->Release();

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthBuffer;
    d3d11Device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

    d3d11Device->CreateDepthStencilView(depthBuffer, nullptr, depthBufferView);

    // release the depth buffer
    depthBuffer->Release();

    return true;
}
DWORD WINAPI ConsoleThread(LPVOID)
{

    while (true)
    {
        // Check for new console output
        char buffer[256];
        DWORD bytesRead;
        while (ReadFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, sizeof(buffer), &bytesRead, NULL))
        {
            if (bytesRead == 0)
                break;

            buffer[bytesRead] = 0;
            console.AddLog(buffer); // Add console output to the ImGui console
        }

        Sleep(100); // Poll for new output periodically
    }

    return 0;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
    {

    }

    LRESULT result = 0;
    switch (msg)
    {


    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        bool isDown = (msg == WM_KEYDOWN);
        if (wparam == VK_ESCAPE)
            DestroyWindow(hwnd);
        else if (wparam == 'W')
            global_keyIsDown[GameActionMoveCamFwd] = isDown;
        else if (wparam == 'A')
            global_keyIsDown[GameActionMoveCamLeft] = isDown;
        else if (wparam == 'S')
            global_keyIsDown[GameActionMoveCamBack] = isDown;
        else if (wparam == 'D')
            global_keyIsDown[GameActionMoveCamRight] = isDown;
        else if (wparam == 'E')
            global_keyIsDown[GameActionRaiseCam] = isDown;
        else if (wparam == 'Q')
            global_keyIsDown[GameActionLowerCam] = isDown;
        else if (wparam == VK_UP)
            global_keyIsDown[GameActionLookUp] = isDown;
        else if (wparam == VK_LEFT)
            global_keyIsDown[GameActionTurnCamLeft] = isDown;
        else if (wparam == VK_DOWN)
            global_keyIsDown[GameActionLookDown] = isDown;
        else if (wparam == VK_RIGHT)
            global_keyIsDown[GameActionTurnCamRight] = isDown;
        break;
    }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
        {
            global_windowDidResize = true;
            break;
        }
        default:
            result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return result;
}



int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ PSTR /*lpCmdLine*/, _In_ int /*nShowCmd*/)
{


    // Open a window
    HWND hwnd;
    {
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"ChronoEngine";
        winClass.lpszMenuName = MAKEINTRESOURCE(IDM_MYMENURESOURCE);

        if (!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        RECT initialRect = { 0, 0, 1920, 1080 };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, TRUE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
            winClass.lpszClassName,
            L"Chrono - untitled [DX11]",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            initialWidth,
            initialHeight,
            0, 0, hInstance, 0);

        if (!hwnd) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }
    // Create D3D11 Device and Context
    ID3D11Device1* d3d11Device;
    ID3D11DeviceContext1* d3d11DeviceContext;
    {
        ID3D11Device* baseDevice;
        ID3D11DeviceContext* baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG_BUILD)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
            0, creationFlags,
            featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, &baseDevice,
            0, &baseDeviceContext);
        if (FAILED(hResult)) {
            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        // Get 1.1 interface of D3D11 Device and Context
        hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&d3d11Device);
        assert(SUCCEEDED(hResult));
        baseDevice->Release();

        hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&d3d11DeviceContext);
        assert(SUCCEEDED(hResult));
        baseDeviceContext->Release();
    }
#ifdef DEBUG_BUILD
    // Set up debug layer to break on D3D11 errors
    ID3D11Debug* d3dDebug = nullptr;
    d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    if (d3dDebug)
    {
        ID3D11InfoQueue* d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }
#endif
    // Create Swap Chain
    IDXGISwapChain1* d3d11SwapChain;
    {
        // Get DXGI Factory (needed to create Swap Chain)
        IDXGIFactory2* dxgiFactory;
        {
            IDXGIDevice1* dxgiDevice;
            HRESULT hResult = d3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
            assert(SUCCEEDED(hResult));

            IDXGIAdapter* dxgiAdapter;
            hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
            assert(SUCCEEDED(hResult));
            dxgiDevice->Release();

            DXGI_ADAPTER_DESC adapterDesc;
            dxgiAdapter->GetDesc(&adapterDesc);

            OutputDebugStringA("Graphics Device: ");
            OutputDebugStringW(adapterDesc.Description);

            hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
            assert(SUCCEEDED(hResult));
            dxgiAdapter->Release();
        }

        DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
        d3d11SwapChainDesc.Width = 0; // use window width
        d3d11SwapChainDesc.Height = 0; // use window height
        d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        d3d11SwapChainDesc.SampleDesc.Count = 1;
        d3d11SwapChainDesc.SampleDesc.Quality = 0;
        d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        d3d11SwapChainDesc.BufferCount = 2;
        d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        d3d11SwapChainDesc.Flags = 0;
      

        HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(d3d11Device, hwnd, &d3d11SwapChainDesc, 0, 0, &d3d11SwapChain);
        assert(SUCCEEDED(hResult));

        dxgiFactory->Release();
    }
    // Create wireframe rasterizer state
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(rasterDesc));
    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.MultisampleEnable = TRUE;
    rasterDesc.AntialiasedLineEnable = TRUE;
    d3d11Device->CreateRasterizerState(&rasterDesc, &rStateWireframe);

    // Create solid rasterizer state
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    d3d11Device->CreateRasterizerState(&rasterDesc, &rStateSolid);

    // Create Render Target and Depth Buffer
    ID3D11RenderTargetView* d3d11FrameBufferView;
    ID3D11DepthStencilView* depthBufferView;
    win32CreateD3D11RenderTargets(d3d11Device, d3d11SwapChain, &d3d11FrameBufferView, &depthBufferView);

    UINT shaderCompileFlags = 0;
    // Compiling with this flag allows debugging shaders with Visual Studio
#if defined(DEBUG_BUILD)
    shaderCompileFlags |= D3DCOMPILE_DEBUG;
#endif

    // Create Vertex Shader for rendering our lights
    ID3DBlob* lightVsCode;
    ID3D11VertexShader* lightVertexShader;
    {
        ID3DBlob* compileErrors;
        HRESULT hResult = D3DCompileFromFile(L"Lights.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", shaderCompileFlags, 0, &lightVsCode, &compileErrors);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (compileErrors) {
                errorString = (const char*)compileErrors->GetBufferPointer();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreateVertexShader(lightVsCode->GetBufferPointer(), lightVsCode->GetBufferSize(), nullptr, &lightVertexShader);
        assert(SUCCEEDED(hResult));
    }
    // Create Pixel Shader for rendering our lights
    ID3D11PixelShader* lightPixelShader;
    {
        ID3DBlob* psBlob;
        ID3DBlob* compileErrors;
        HRESULT hResult = D3DCompileFromFile(L"Lights.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", shaderCompileFlags, 0, &psBlob, &compileErrors);
        if(FAILED(hResult))
        {
            const char* errorString = NULL;
            if(hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if(compileErrors){
                errorString = (const char*)compileErrors->GetBufferPointer();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &lightPixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }
    // Create Input Layout for our light vertex shader
    ID3D11InputLayout* lightInputLayout;
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), lightVsCode->GetBufferPointer(), lightVsCode->GetBufferSize(), &lightInputLayout);
        assert(SUCCEEDED(hResult));
        lightVsCode->Release();
    }
    // Create Vertex Shader for rendering our lit objects
    ID3DBlob* blinnPhongVsCode;
    ID3D11VertexShader* blinnPhongVertexShader;
    {
        ID3DBlob* compileErrors;
        HRESULT hResult = D3DCompileFromFile(L"BlinnPhong.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", shaderCompileFlags, 0, &blinnPhongVsCode, &compileErrors);
        if(FAILED(hResult))
        {
            const char* errorString = NULL;
            if(hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if(compileErrors){
                errorString = (const char*)compileErrors->GetBufferPointer();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreateVertexShader(blinnPhongVsCode->GetBufferPointer(), blinnPhongVsCode->GetBufferSize(), nullptr, &blinnPhongVertexShader);
        assert(SUCCEEDED(hResult));
    }
    // Create Pixel Shader for rendering our lit objects
    ID3D11PixelShader* blinnPhongPixelShader;
    {
        ID3DBlob* psBlob;
        ID3DBlob* compileErrors;
        HRESULT hResult = D3DCompileFromFile(L"BlinnPhong.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", shaderCompileFlags, 0, &psBlob, &compileErrors);
        if(FAILED(hResult))
        {
            const char* errorString = NULL;
            if(hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if(compileErrors){
                errorString = (const char*)compileErrors->GetBufferPointer();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &blinnPhongPixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }
    // Create Input Layout for our Blinn-Phong vertex shader
    ID3D11InputLayout* blinnPhongInputLayout;
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), blinnPhongVsCode->GetBufferPointer(), blinnPhongVsCode->GetBufferSize(), &blinnPhongInputLayout);
        assert(SUCCEEDED(hResult));
        blinnPhongVsCode->Release();
    }

    // Create Vertex and Index Buffer
    ID3D11Buffer* cubeVertexBuffer;
    ID3D11Buffer* cubeIndexBuffer;
    ID3D11Buffer* axisVertexBuffer;
    ID3D11Buffer* axisIndexBuffer;
    UINT cubeNumIndices;
    UINT cubeStride;
    UINT cubeOffset;
    UINT axisNumIndices;
    UINT axisStride;
    UINT axisOffset;
    {
        LoadedObj obj = loadObj("cube.obj");
        LoadedObj axis = loadObj("axis.obj");
        cubeStride = sizeof(VertexData);
        cubeOffset = 0;
        cubeNumIndices = obj.numIndices;
        axisStride = sizeof(VertexData);
        axisOffset = 0;
        axisNumIndices = axis.numIndices;

        // Cube Vertex Buffer
        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = obj.numVertices * sizeof(VertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { obj.vertexBuffer };

        HRESULT hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &cubeVertexBuffer);
        assert(SUCCEEDED(hResult));

        // Axis Vertex Buffer
        vertexBufferDesc.ByteWidth = axis.numVertices * sizeof(VertexData);
        vertexSubresourceData.pSysMem = axis.vertexBuffer;

        hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &axisVertexBuffer);
        assert(SUCCEEDED(hResult));

        // Cube Index Buffer
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = obj.numIndices * sizeof(uint16_t);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { obj.indexBuffer };

        hResult = d3d11Device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &cubeIndexBuffer);
        assert(SUCCEEDED(hResult));

        // Axis Index Buffer
        indexBufferDesc.ByteWidth = axis.numIndices * sizeof(uint16_t);
        indexSubresourceData.pSysMem = axis.indexBuffer;

        hResult = d3d11Device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &axisIndexBuffer);
        assert(SUCCEEDED(hResult));

        freeLoadedObj(obj);
        freeLoadedObj(axis);
    }

    // Create Sampler State
    ID3D11SamplerState* samplerState;
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        d3d11Device->CreateSamplerState(&samplerDesc, &samplerState);
    }
    
    // Load Image
    int texWidth, texHeight, texNumChannels;
    int texForceNumChannels = 4;
    unsigned char* testTextureBytes = stbi_load("crate.png", &texWidth, &texHeight,
                                                &texNumChannels, texForceNumChannels);
    assert(testTextureBytes);
    int texBytesPerRow = 4 * texWidth;

    // Create Texture
    ID3D11ShaderResourceView* textureView;
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width              = texWidth;
        textureDesc.Height             = texHeight;
        textureDesc.MipLevels          = 1;
        textureDesc.ArraySize          = 1;
        textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count   = 1;
        textureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = testTextureBytes;
        textureSubresourceData.SysMemPitch = texBytesPerRow;

        ID3D11Texture2D* texture;
        d3d11Device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        d3d11Device->CreateShaderResourceView(texture, nullptr, &textureView);
        texture->Release();
    }

    free(testTextureBytes);

    // Create Constant Buffer for our light vertex shader
    struct LightVSConstants
    {
        float4x4 modelViewProj;
        float4 color;
    };
    ID3D11Buffer* lightVSConstantBuffer;
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth      = sizeof(LightVSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = d3d11Device->CreateBuffer(&constantBufferDesc, nullptr, &lightVSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }
    // Create Constant Buffer for our Blinn-Phong vertex shader
    struct BlinnPhongVSConstants
    {
        float4x4 modelViewProj;
        float4x4 modelView;
        float3x3 normalMatrix;
    };
    ID3D11Buffer* blinnPhongVSConstantBuffer;
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth      = sizeof(BlinnPhongVSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = d3d11Device->CreateBuffer(&constantBufferDesc, nullptr, &blinnPhongVSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }
    struct DirectionalLight // Directional Light
    {
        float4 dirEye; //NOTE: Direction towards the light
        float4 color;
    };
    struct PointLight // Point Light
    {
        float4 posEye;
        float4 color;
    };
    // Create Constant Buffer for our Blinn-Phong pixel shader
    struct BlinnPhongPSConstants
    {
        DirectionalLight dirLight;
        PointLight pointLights[50];
    };

    ID3D11Buffer* blinnPhongPSConstantBuffer;
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth      = sizeof(BlinnPhongPSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = d3d11Device->CreateBuffer(&constantBufferDesc, nullptr, &blinnPhongPSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }

    ID3D11RasterizerState* rasterizerState;
    {
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = TRUE;

        d3d11Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    }

    ID3D11DepthStencilState* depthStencilState;
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable    = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;

        d3d11Device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    }

    // Camera
    static float3 cameraPos = {0, 0, 2};
    float3 cameraFwd = {0, 0, -1};
    static float cameraPitch = 0.f;
    static float cameraYaw = 0.f;

    static float vec4a[4];

    float4x4 perspectiveMat = {};
    global_windowDidResize = true; // To force initial perspectiveMat calculation

    // Timing
    LONGLONG startPerfCount = 0;
    LONGLONG perfCounterFrequency = 0;
    {
        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount);
        startPerfCount = perfCount.QuadPart;
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);
        perfCounterFrequency = perfFreq.QuadPart;
    }
    double currentTimeInSeconds = 0.0;

    IMGUI_CHECKVERSION(); // check the imgui version
    ImGui::CreateContext(); // create the context
    ImGuiIO& io = ImGui::GetIO(); (void)io; // get the Input/Output
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    io.DisplayFramebufferScale = ImVec2(1920, 1080); // Set appropriate scale factors

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(d3d11Device, d3d11DeviceContext);

#pragma region Chrono Style
    ImGuiStyle& chronoDark = ImGui::GetStyle();
    chronoDark.WindowRounding = 6.45f;
    chronoDark.FrameRounding = 3.9f;
    chronoDark.ScrollbarRounding = 1;

    // Modify the colors for the dark theme
    chronoDark.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    chronoDark.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    chronoDark.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    chronoDark.Colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.90f);
    chronoDark.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.00f);
    chronoDark.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    chronoDark.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    chronoDark.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 0.40f);
    chronoDark.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.45f);
    chronoDark.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    chronoDark.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    chronoDark.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    chronoDark.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.80f);
    chronoDark.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    chronoDark.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
    chronoDark.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    chronoDark.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
    chronoDark.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    chronoDark.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
    chronoDark.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
    chronoDark.Colors[ImGuiCol_Button] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    chronoDark.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    chronoDark.Colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    chronoDark.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
    chronoDark.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
    chronoDark.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
    chronoDark.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.60f, 0.85f);
    chronoDark.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.60f);
    chronoDark.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.70f, 0.70f, 0.90f);
    chronoDark.Colors[ImGuiCol_PlotLines] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    chronoDark.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.80f, 0.70f, 0.00f, 1.00f);
    chronoDark.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.80f, 0.70f, 0.00f, 1.00f);
    chronoDark.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    chronoDark.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
#pragma endregion


    // Start a separate thread to capture console output
    CreateThread(NULL, 0, ConsoleThread, NULL, 0, NULL);

    cout << "Chrono Console Started!";

    LONG lReg;
    HKEY hKey;
    lReg = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        L"Software\\Chrono Engine",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL);
    if (lReg != ERROR_SUCCESS)
    {
        cout << "Registry creation failed & Error No - " << GetLastError() << endl;
    }
    cout << "Registry Creation Success!" << endl;
   
    RegCloseKey(hKey);

    // Main Loop
    bool isRunning = true;
    while(isRunning)
    {
        if (RobotoFont)
        {
            SystemFont = false;
            io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 15); // we update the font every frame.
        }
        if (SystemFont)
        {
            RobotoFont = false;
            io.Fonts->AddFontFromFileTTF("System.ttf", 15); // we update the font every frame.
        }
        float dt;
        {
            double previousTimeInSeconds = currentTimeInSeconds;
            LARGE_INTEGER perfCount;
            QueryPerformanceCounter(&perfCount);

            currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
            dt = (float)(currentTimeInSeconds - previousTimeInSeconds);
            if(dt > (1.f / 60.f))
                dt = (1.f / 60.f);
        }

        MSG msg = {};
        while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                isRunning = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        // Get window dimensions
        int windowWidth, windowHeight;
        float windowAspectRatio;
        {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            windowWidth = clientRect.right - clientRect.left;
            windowHeight = clientRect.bottom - clientRect.top;
            windowAspectRatio = (float)windowWidth / (float)windowHeight;
        }

        if(global_windowDidResize)
        {
            d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
            d3d11FrameBufferView->Release();
            depthBufferView->Release();

            HRESULT res = d3d11SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            assert(SUCCEEDED(res));
            
            win32CreateD3D11RenderTargets(d3d11Device, d3d11SwapChain, &d3d11FrameBufferView, &depthBufferView);
            perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(84), 0.1f, 1000.f);

            global_windowDidResize = false;
        }

        // Update camera
        {
            float3 camFwdXZ = normalise(float3{cameraFwd.x, 0, cameraFwd.z});
            float3 cameraRightXZ = cross(camFwdXZ, {0, 1, 0});

            const float CAM_MOVE_SPEED = 5.f; // in metres per second
            const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
            if(global_keyIsDown[GameActionMoveCamFwd])
                cameraPos += camFwdXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionMoveCamBack])
                cameraPos -= camFwdXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionMoveCamLeft])
                cameraPos -= cameraRightXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionMoveCamRight])
                cameraPos += cameraRightXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionRaiseCam])
                cameraPos.y += CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionLowerCam])
                cameraPos.y -= CAM_MOVE_AMOUNT;
            
            const float CAM_TURN_SPEED = M_PI; // in radians per second
            const float CAM_TURN_AMOUNT = CAM_TURN_SPEED * dt;
            if(global_keyIsDown[GameActionTurnCamLeft])
                cameraYaw += CAM_TURN_AMOUNT;
            if(global_keyIsDown[GameActionTurnCamRight])
                cameraYaw -= CAM_TURN_AMOUNT;
            if(global_keyIsDown[GameActionLookUp])
                cameraPitch += CAM_TURN_AMOUNT;
            if(global_keyIsDown[GameActionLookDown])
                cameraPitch -= CAM_TURN_AMOUNT;

            // Wrap yaw to avoid floating-point errors if we turn too far
            while(cameraYaw >= 2*M_PI) 
                cameraYaw -= 2*M_PI;
            while(cameraYaw <= -2*M_PI) 
                cameraYaw += 2*M_PI;

            // Clamp pitch to stop camera flipping upside down
            if(cameraPitch > degreesToRadians(85)) 
                cameraPitch = degreesToRadians(85);
            if(cameraPitch < -degreesToRadians(85)) 
                cameraPitch = -degreesToRadians(85);
        }

        // Calculate view matrix from camera data
        // 
        // float4x4 viewMat = inverse(rotateXMat(cameraPitch) * rotateYMat(cameraYaw) * translationMat(cameraPos));
        // NOTE: We can simplify this calculation to avoid inverse()!
        // Applying the rule inverse(A*B) = inverse(B) * inverse(A) gives:
        // float4x4 viewMat = inverse(translationMat(cameraPos)) * inverse(rotateYMat(cameraYaw)) * inverse(rotateXMat(cameraPitch));
        // The inverse of a rotation/translation is a negated rotation/translation:
        float4x4 viewMat = translationMat(-cameraPos) * rotateYMat(-cameraYaw) * rotateXMat(-cameraPitch);
        float4x4 inverseViewMat = rotateXMat(cameraPitch) * rotateYMat(cameraYaw) * translationMat(cameraPos);
        // Update the forward vector we use for camera movement:
        cameraFwd = {-viewMat.m[2][0], -viewMat.m[2][1], -viewMat.m[2][2]};

        // Calculate matrices for cubes
        static const int  NUM_CUBES = 4;
        float4x4 cubeModelViewMats[NUM_CUBES];
        float3x3 cubeNormalMats[NUM_CUBES];
        {
            float3 cubePositions[NUM_CUBES] = {
                {0.f, 0.f, 0.f},
                {-3.f, 0.f, -1.5f},
                {4.5f, 0.2f, -3.f},
                {0.f, -1.5f, 0.f},
            };

            float modelXRotation = 0.f; //*(float)(M_PI* currentTimeInSeconds);
            float modelYRotation = 0.f; // *(float)(M_PI* currentTimeInSeconds);
            for(int i=0; i<NUM_CUBES; ++i)
            {

               // modelXRotation += 0.6f*i; // Add an offset so cubes have different phases
               // modelYRotation += 0.6f*i;
                float4x4 modelMat = rotateXMat(modelXRotation) * rotateYMat(modelYRotation) * translationMat(cubePositions[i]);
                float4x4 inverseModelMat = translationMat(-cubePositions[i]) * rotateYMat(-modelYRotation) * rotateXMat(-modelXRotation);
                cubeModelViewMats[i] = modelMat * viewMat;
                float4x4 inverseModelViewMat = inverseViewMat * inverseModelMat;
                cubeNormalMats[i] = float4x4ToFloat3x3(transpose(inverseModelViewMat));
                cout << "Made a cube!";
            }

        }

        // Move the point lights
        const int NUM_LIGHTS = 1;
        static float brightness = 1.5f;
        float4 lightColor[NUM_LIGHTS] = {
            {brightness, brightness, brightness, brightness}
           // {0.9f, 0.1f, 0.6f, 1.f}
        };
        float4x4 lightModelViewMats[NUM_LIGHTS];
        float4 pointLightPosEye[NUM_LIGHTS];
        {
            float4 initialPointLightPositions[NUM_LIGHTS] = {
                {1, 0.5f, 0, 1}
                //{-1, 0.7f, -1.2f, 1}
            };

            float lightRotation = -0.3f; //* (float)(M_PI * currentTimeInSeconds);
            for(int i=0; i<NUM_LIGHTS; ++i)
            {
                lightRotation += 0.5f*i; // Add an offset so lights have different phases
                                        
                lightModelViewMats[i] = scaleMat(0.2f) * translationMat(initialPointLightPositions[i].xyz) * rotateYMat(lightRotation) * viewMat;
                pointLightPosEye[i] = lightModelViewMats[i].cols[3];
                cout << "Made a light with brightness: " << brightness;
            }
        }

        //calculate custom models/ added
        // Calculate matrices for cubes
        const int NUM_OTHERS = 1;
        float4x4 otherModelViewMats[NUM_OTHERS];
        float3x3 otherNormalMats[NUM_OTHERS];
        {
            float3 otherPositions[NUM_OTHERS] = {
               {0.f, 0.f, 0.f},// we will add it to the positions when we create one.
            };

            float modelXRotation = 0.f; //*(float)(M_PI* currentTimeInSeconds);
            float modelYRotation = 0.f; // *(float)(M_PI* currentTimeInSeconds);
            for (int i = 0; i < NUM_OTHERS; ++i)
            {
                // modelXRotation += 0.6f*i; // Add an offset so cubes have different phases
                // modelYRotation += 0.6f*i;
                float4x4 modelMat = rotateXMat(modelXRotation) * rotateYMat(modelYRotation) * translationMat(otherPositions[i]);
                float4x4 inverseModelMat = translationMat(-otherPositions[i]) * rotateYMat(-modelYRotation) * rotateXMat(-modelXRotation);
                otherModelViewMats[i] = modelMat * viewMat;
                float4x4 inverseModelViewMat = inverseViewMat * inverseModelMat;
                otherNormalMats[i] = float4x4ToFloat3x3(transpose(inverseModelViewMat));
                cout << "Made a object with type (other).";
            }
        }
        // Data for cubes
        struct CubeData {
            XMMATRIX modelMatrix;
            XMMATRIX inverseModelMatrix;
            XMMATRIX normalMatrix;
        };

        CubeData cubes[NUM_CUBES];

        static FLOAT defaultbackgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        static FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);
        
        d3d11DeviceContext->ClearDepthStencilView(depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)windowWidth, (FLOAT)windowHeight, 0.0f, 1.0f };
        d3d11DeviceContext->RSSetViewports(1, &viewport);

        d3d11DeviceContext->RSSetState(rasterizerState);
        d3d11DeviceContext->OMSetDepthStencilState(depthStencilState, 0);

        d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, depthBufferView);

        d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        d3d11DeviceContext->IASetVertexBuffers(0, 1, &cubeVertexBuffer, &cubeStride, &cubeOffset);
        d3d11DeviceContext->IASetIndexBuffer(cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // Draw lights
        {
            d3d11DeviceContext->IASetInputLayout(lightInputLayout);
            d3d11DeviceContext->VSSetShader(lightVertexShader, nullptr, 0);
            d3d11DeviceContext->PSSetShader(lightPixelShader, nullptr, 0);
            d3d11DeviceContext->VSSetConstantBuffers(0, 1, &lightVSConstantBuffer);

            for(int i=0; i<NUM_LIGHTS; ++i){
                // Update vertex shader constant buffer
                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                d3d11DeviceContext->Map(lightVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                LightVSConstants* constants = (LightVSConstants*)(mappedSubresource.pData);
                constants->modelViewProj = lightModelViewMats[i] * perspectiveMat;
                constants->color = lightColor[i];
                d3d11DeviceContext->Unmap(lightVSConstantBuffer, 0);

                d3d11DeviceContext->DrawIndexed(cubeNumIndices, 0, 0);
            }
        }

        //Draw Imgui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Draw cubes
        {
            d3d11DeviceContext->IASetInputLayout(blinnPhongInputLayout);
            d3d11DeviceContext->VSSetShader(blinnPhongVertexShader, nullptr, 0);
            d3d11DeviceContext->PSSetShader(blinnPhongPixelShader, nullptr, 0);

            d3d11DeviceContext->PSSetShaderResources(0, 1, &textureView);
            d3d11DeviceContext->PSSetSamplers(0, 1, &samplerState);

            d3d11DeviceContext->VSSetConstantBuffers(0, 1, &blinnPhongVSConstantBuffer);
            d3d11DeviceContext->PSSetConstantBuffers(0, 1, &blinnPhongPSConstantBuffer);

            // Update pixel shader constant buffer
            {
                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                d3d11DeviceContext->Map(blinnPhongPSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                BlinnPhongPSConstants* constants = (BlinnPhongPSConstants*)(mappedSubresource.pData);
                constants->dirLight.dirEye = normalise(float4{1.f, 1.f, 1.f, 0.f});
                constants->dirLight.color = {0.7f, 0.8f, 0.2f, 1.f};
                for(int i=0; i<NUM_LIGHTS; ++i){
                    constants->pointLights[i].posEye = pointLightPosEye[i];
                    constants->pointLights[i].color = lightColor[i];
                }
                d3d11DeviceContext->Unmap(blinnPhongPSConstantBuffer, 0);
            }

            for(int i=0; i<NUM_CUBES; ++i)
            {
                // Update vertex shader constant buffer
                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                d3d11DeviceContext->Map(blinnPhongVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                BlinnPhongVSConstants* constants = (BlinnPhongVSConstants*)(mappedSubresource.pData);
                constants->modelViewProj = cubeModelViewMats[i] * perspectiveMat;
                constants->modelView = cubeModelViewMats[i];
                constants->normalMatrix = cubeNormalMats[i];
                d3d11DeviceContext->Unmap(blinnPhongVSConstantBuffer, 0);

                d3d11DeviceContext->DrawIndexed(cubeNumIndices, 0, 0);
            }
        }

        float cameraX = cameraPos.x;
        float cameraY = cameraPos.y;
        float cameraZ = cameraPos.z;

        float camPitch = cameraPitch;
        float camYaw = cameraYaw;
        


        static bool useSkybox = false;

        cout << "Start windows:";

        ImGui::ShowStyleSelector("Style Selector");
        cout << "style selector window initalized!";
        ImGui::Begin("Chrono Engine Scene");
        cout << "scene settings window initalized";
        ImGui::Text("Camera Position:");
        ImGui::InputFloat("X: %.3f", &cameraX, 1.0f, 2.0f);
        ImGui::InputFloat("Y: %.3f", &cameraY, 1.0f, 2.0f);
        ImGui::InputFloat("Z: %.3f", &cameraZ, 1.0f, 2.0f);
        ImGui::Text("Camera Rotation:");
        ImGui::InputFloat("Pitch: %.3f", &camPitch, 1.0f, 2.0f);
        ImGui::InputFloat("Yaw: %.3f", &camYaw, 1.0f, 2.0f);
        ImGui::NewLine();
        #pragma region Scene Brightness
        ImGui::Text("Scene Brightness:");
        ImGui::InputFloat("", &brightness, 1.0f, 5.00f);
        #pragma endregion

        #pragma region Scene Sky
        ImGui::Text("Scene Sky:");
        if (ImGui::Checkbox("Use Skybox", &useSkybox))
        {
            if (ImGui::Button("Choose Skybox"))
            {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                    COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileOpen;

                    // Create the FileOpenDialog object.
                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                    if (SUCCEEDED(hr))
                    {
                        // Show the Open dialog box.
                        hr = pFileOpen->Show(NULL);

                        // Get the file name from the dialog box.
                        if (SUCCEEDED(hr))
                        {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                                // Display the file name to the user.
                                if (SUCCEEDED(hr))
                                {
                                    MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                                    CoTaskMemFree(pszFilePath);
                                }
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                    CoUninitialize();
                }
            }
        }
        ImGui::InputFloat("Red", &backgroundColor[0], 1.0f, 2.0f);
        ImGui::InputFloat("Green", &backgroundColor[1], 1.0f, 2.0f);
        ImGui::InputFloat("Blue", &backgroundColor[2], 1.0f, 2.0f);
#pragma endregion

        int selectedCube = -1; // Variable to store the selected cube index

        // ImGui window for the hierarchy
        ImGui::Begin("Hierarchy");
        cout << "hierachy window initalized!";

        for (int i = 0; i < NUM_CUBES; ++i) {
            // Selectable cube in the hierarchy
            ImGui::PushID(i);
            if (ImGui::Selectable("Cube", selectedCube == i)) {
                selectedCube = i;
            }
            ImGui::PopID();
        }
        ImGui::End();

        // ImGui window for cube properties
        ImGui::Begin("Properties");
        cout << "properties window initalized!";

        if (selectedCube >= 0) {
            // Display the name and stats of the selected cube
            ImGui::Text("Selected Cube: %d", selectedCube);
           
            // Add more stats or properties as needed
        }
        else {
            ImGui::Text("No cube selected.");
        }

        ImGui::End();

        if (ImGui::Button("Create new Object"))
        {
           
        }

     // ImGui::ShowDemoWindow(); // if you want demo window.

        // Inside your ImGui rendering code
        static char saveFileName[128] = "scene.cproj";
        static char loadFileName[128] = "scene.cproj";

        ImGui::BeginMainMenuBar();
        cout << "Start menu Bar.";
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Project Settings")) {
                    showProjectSettingsWindow = true;
                }
                if (ImGui::MenuItem("Editor Settings")) {
                    showEditorSettingsWindow = true;
                }
                if (ImGui::MenuItem("Save"))
                {
                    
                }
                ImGui::MenuItem("Save As..");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("WireFrame"))
                {
                    d3d11DeviceContext->RSSetState(rStateWireframe);
                }
                if(ImGui::MenuItem("Solid"))
                {
                    d3d11DeviceContext->RSSetState(rStateSolid);
                }
             
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Documentation"))
                {
                    ShellExecute(0, 0, L"https://github.com/NanoDogs-Studios/ChronoEngine/wiki", 0, 0, SW_SHOW);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Misc"))
            {
                if (ImGui::MenuItem("Quit"))
                {
                    quick_exit(0);
                }
                if (ImGui::MenuItem("Fullscreen"))
                {
                    d3d11SwapChain->SetFullscreenState(true, NULL);
                    
                    MessageBoxA(hwnd, "You are now in fullscreen, press ESCAPE to quit Chrono.", "Chrono Warning!", MB_OK);
                    
                }
                if (ImGui::MenuItem("Windowed"))
                {
                    d3d11SwapChain->SetFullscreenState(false, NULL);
                }
                if (ImGui::MenuItem("Save Layout"))
                {
                    ImGui::SaveIniSettingsToDisk("imgui.ini");
                }
                if (ImGui::MenuItem("Reset Layout"))
                {
                    ImGui::LoadIniSettingsFromDisk("defaultLayout.ini");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::End();

        ImGui::Begin("Performance / Debug");
        cout << "Performance / Debug window initalized!";
        {
            cout << "Reading info...";
            ImGui::Text("%.3f m/s | %.1f FPS", 1000.0f / io.Framerate, io.Framerate);
            // Get and display the operating system version
            // note: NTDDI_WIN11 does not exist as of oct 2023 so we just say it is either 10 or 11
            if (IsWindowsVersionOrGreater(HIBYTE(NTDDI_WIN10), LOBYTE(NTDDI_WIN10), 0)) {
                ImGui::Text("Operating System: Windows 10/11");
            }
            else if (IsWindowsVersionOrGreater(6, 2, 0)) {
                ImGui::Text("Operating System: Windows 8/8.1");
            }
            else if (IsWindowsVersionOrGreater(6, 1, 0)) {
                ImGui::Text("Operating System: Windows 7");
            }
            else if (IsWindowsVersionOrGreater(6, 0, 0)) {
                ImGui::Text("Operating System: Windows Vista");
            }
            else {
                ImGui::Text("Operating System: Unknown");
            }

            // Get and display the computer name
            char computerName[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
            GetComputerNameA(computerName, &size);
            ImGui::Text("Computer Name: %s", computerName);

            // Get and display the current user name
            char username[MAX_PATH];
            DWORD usernameSize = MAX_PATH;
            GetUserNameA(username, &usernameSize);
            ImGui::Text("Current User: %s", username);

            // Get and display system memory (RAM) information
            MEMORYSTATUSEX memoryStatus;
            memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
            GlobalMemoryStatusEx(&memoryStatus);
            ImGui::Text("System Memory (RAM): %I64u MB", memoryStatus.ullTotalPhys / (1024 * 1024));
        }
        if (showProjectSettingsWindow) {
            RenderProjectSettingsWindow();
        }

        if (showEditorSettingsWindow) {
            RenderEditorSettingsWindow();
        }

        // Render your ImGui content
        if (ImGui::Begin("Console"))
            cout << "console window initalized!";
        {
            console.Draw("Console"); // Draw the console window
        }

        ImGui::Begin("Project Assets");
        if (ImGui::Button("Add Item to project"))
        {

            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr)) {
                IFileOpenDialog* pFileOpen;
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr)) {
                    hr = pFileOpen->Show(NULL);

                    if (SUCCEEDED(hr)) {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr)) {
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            if (SUCCEEDED(hr)) {
                                // Copy the selected file path
                                wcscpy_s(selectedFilePath, MAX_PATH, pszFilePath);
                                projectAssets.push_back(selectedFilePath); // Add to the project assets
                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                    pFileOpen->Release();
                }
                CoUninitialize();
            }
        }

       


        /// Display the selected file paths in a grid
        ImGui::Columns(3, nullptr, false);
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Path");
        ImGui::NextColumn();
        ImGui::Text(""); // Empty header for the third column
        ImGui::NextColumn();

        for (size_t i = 0; i < projectAssets.size(); ++i) {
            ImGui::Text("File %zu", i + 1);
            ImGui::NextColumn();
            ImGui::Text("%S", projectAssets[i].c_str());
            ImGui::NextColumn();
            if (ImGui::Button("Open")) {
                // Handle opening the file here
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                // Handle deleting the file here
                if (i < projectAssets.size()) {
                    // Erase the file from the vector
                    projectAssets.erase(projectAssets.begin() + i);
                }
            }
            ImGui::NextColumn();
        }
        ImGui::Columns(1);





        ImGui::End();
        cout << "End Gui";

        ImGui::Render();
        cout << "Render Gui";
       // d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        d3d11SwapChain->Present(0, 0);
        cout << "d3d11SwapChain presented with 0, 0.";

    }

    // Cleanup and exit
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cout << "Shutting down ImGui";
    
    return 0;
}

