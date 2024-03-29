#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include "implot.h"
#include "implot_internal.h"

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif


#include <vector>
#include <string>
#include <iostream>

#include "user_interface_window.h"
#include "resources/resources.h"

// Dear ImGui stuff
struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12Fence* g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace dnf_composer
{
	namespace user_interface
	{
		class UserInterface
		{
		private:
			std::vector<std::shared_ptr<UserInterfaceWindow>> windows;

			HWND windowHandle;
			WNDCLASSEXW windowClass;
			bool closeUI;

			ImVec4 clear_color = ImVec4(0.2f, 0.2f, 0.2f, 1.00f); // Darkish gray
		public:
			UserInterface();
			UserInterface(const UserInterface&) = delete;
			UserInterface& operator=(const UserInterface&) = delete;
			UserInterface(UserInterface&&) = delete;
			UserInterface& operator=(UserInterface&&) = delete;

			void init();
			void step();
			void close() const;

			void activateWindow(const std::shared_ptr<UserInterfaceWindow>& window);

			bool getCloseUI() const;
			~UserInterface() = default;

		private:
			void render() const;
		};
	}
}