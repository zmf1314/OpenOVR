#pragma once
#include "OpenVR/interfaces/IVRCompositor_020.h"
#include "BaseCompositor.h"

namespace CVRCompositor_020_scope {

	// FIXME don't use namespaces like this in a header!
	using namespace vr;
	using namespace IVRCompositor_020;

	class CVRCompositor_020 : IVRCompositor_020::IVRCompositor {
	private:
		BaseCompositor base;

	public:
		CVRCompositor_020();
		~CVRCompositor_020();

		/** Sets tracking space returned by WaitGetPoses */
		virtual void SetTrackingSpace(ETrackingUniverseOrigin eOrigin);

		/** Gets current tracking space returned by WaitGetPoses */
		virtual ETrackingUniverseOrigin GetTrackingSpace();

		/** Scene applications should call this function to get poses to render with (and optionally poses predicted an additional frame out to use for gameplay).
		* This function will block until "running start" milliseconds before the start of the frame, and should be called at the last moment before needing to
		* start rendering.
		*
		* Return codes:
		*	- IsNotSceneApplication (make sure to call VR_Init with VRApplicaiton_Scene)
		*	- DoNotHaveFocus (some other app has taken focus - this will throttle the call to 10hz to reduce the impact on that app)
		*/
		virtual EVRCompositorError WaitGetPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
			VR_ARRAY_COUNT(unGamePoseArrayCount) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);

		/** Get the last set of poses returned by WaitGetPoses. */
		virtual EVRCompositorError GetLastPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
			VR_ARRAY_COUNT(unGamePoseArrayCount) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);

		/** Interface for accessing last set of poses returned by WaitGetPoses one at a time.
		* Returns VRCompositorError_IndexOutOfRange if unDeviceIndex not less than k_unMaxTrackedDeviceCount otherwise VRCompositorError_None.
		* It is okay to pass NULL for either pose if you only want one of the values. */
		virtual EVRCompositorError GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t *pOutputPose, TrackedDevicePose_t *pOutputGamePose);

		/** Updated scene texture to display. If pBounds is NULL the entire texture will be used.  If called from an OpenGL app, consider adding a glFlush after
		* Submitting both frames to signal the driver to start processing, otherwise it may wait until the command buffer fills up, causing the app to miss frames.
		*
		* OpenGL dirty state:
		*	glBindTexture
		*
		* Return codes:
		*	- IsNotSceneApplication (make sure to call VR_Init with VRApplicaiton_Scene)
		*	- DoNotHaveFocus (some other app has taken focus)
		*	- TextureIsOnWrongDevice (application did not use proper AdapterIndex - see IVRSystem.GetDXGIOutputInfo)
		*	- SharedTexturesNotSupported (application needs to call CreateDXGIFactory1 or later before creating DX device)
		*	- TextureUsesUnsupportedFormat (scene textures must be compatible with DXGI sharing rules - e.g. uncompressed, no mips, etc.)
		*	- InvalidTexture (usually means bad arguments passed in)
		*	- AlreadySubmitted (app has submitted two left textures or two right textures in a single frame - i.e. before calling WaitGetPoses again)
		*/
		virtual EVRCompositorError Submit(EVREye eEye, const Texture_t *pTexture, const VRTextureBounds_t* pBounds, EVRSubmitFlags nSubmitFlags = Submit_Default);

		/** Clears the frame that was sent with the last call to Submit. This will cause the
		* compositor to show the grid until Submit is called again. */
		virtual void ClearLastSubmittedFrame();

		/** Call immediately after presenting your app's window (i.e. companion window) to unblock the compositor.
		* This is an optional call, which only needs to be used if you can't instead call WaitGetPoses immediately after Present.
		* For example, if your engine's render and game loop are not on separate threads, or blocking the render thread until 3ms before the next vsync would
		* introduce a deadlock of some sort.  This function tells the compositor that you have finished all rendering after having Submitted buffers for both
		* eyes, and it is free to start its rendering work.  This should only be called from the same thread you are rendering on. */
		virtual void PostPresentHandoff();

		/** Returns true if timing data is filled it.  Sets oldest timing info if nFramesAgo is larger than the stored history.
		* Be sure to set timing.size = sizeof(Compositor_FrameTiming) on struct passed in before calling this function. */
		virtual bool GetFrameTiming(Compositor_FrameTiming *pTiming, uint32_t unFramesAgo);

		/** Interface for copying a range of timing data.  Frames are returned in ascending order (oldest to newest) with the last being the most recent frame.
		* Only the first entry's m_nSize needs to be set, as the rest will be inferred from that.  Returns total number of entries filled out. */
		virtual uint32_t GetFrameTimings(Compositor_FrameTiming *pTiming, uint32_t nFrames);

		/** Returns the time in seconds left in the current (as identified by FrameTiming's frameIndex) frame.
		* Due to "running start", this value may roll over to the next frame before ever reaching 0.0. */
		virtual float GetFrameTimeRemaining();

		/** Fills out stats accumulated for the last connected application.  Pass in sizeof( Compositor_CumulativeStats ) as second parameter. */
		virtual void GetCumulativeStats(Compositor_CumulativeStats *pStats, uint32_t nStatsSizeInBytes);

		/** Fades the view on the HMD to the specified color. The fade will take fSeconds, and the color values are between
		* 0.0 and 1.0. This color is faded on top of the scene based on the alpha parameter. Removing the fade color instantly
		* would be FadeToColor( 0.0, 0.0, 0.0, 0.0, 0.0 ).  Values are in un-premultiplied alpha space. */
		virtual void FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground = false);

		/** Get current fade color value. */
		virtual HmdColor_t GetCurrentFadeColor(bool bBackground = false);

		/** Fading the Grid in or out in fSeconds */
		virtual void FadeGrid(float fSeconds, bool bFadeIn);

		/** Get current alpha value of grid. */
		virtual float GetCurrentGridAlpha();

		/** Override the skybox used in the compositor (e.g. for during level loads when the app can't feed scene images fast enough)
		* Order is Front, Back, Left, Right, Top, Bottom.  If only a single texture is passed, it is assumed in lat-long format.
		* If two are passed, it is assumed a lat-long stereo pair. */
		virtual EVRCompositorError SetSkyboxOverride(VR_ARRAY_COUNT(unTextureCount) const Texture_t *pTextures, uint32_t unTextureCount);

		/** Resets compositor skybox back to defaults. */
		virtual void ClearSkyboxOverride();

		/** Brings the compositor window to the front. This is useful for covering any other window that may be on the HMD
		* and is obscuring the compositor window. */
		virtual void CompositorBringToFront();

		/** Pushes the compositor window to the back. This is useful for allowing other applications to draw directly to the HMD. */
		virtual void CompositorGoToBack();

		/** Tells the compositor process to clean up and exit. You do not need to call this function at shutdown. Under normal
		* circumstances the compositor will manage its own life cycle based on what applications are running. */
		virtual void CompositorQuit();

		/** Return whether the compositor is fullscreen */
		virtual bool IsFullscreen();

		/** Returns the process ID of the process that is currently rendering the scene */
		virtual uint32_t GetCurrentSceneFocusProcess();

		/** Returns the process ID of the process that rendered the last frame (or 0 if the compositor itself rendered the frame.)
		* Returns 0 when fading out from an app and the app's process Id when fading into an app. */
		virtual uint32_t GetLastFrameRenderer();

		/** Returns true if the current process has the scene focus */
		virtual bool CanRenderScene();

		/** Creates a window on the primary monitor to display what is being shown in the headset. */
		virtual void ShowMirrorWindow();

		/** Closes the mirror window. */
		virtual void HideMirrorWindow();

		/** Returns true if the mirror window is shown. */
		virtual bool IsMirrorWindowVisible();

		/** Writes all images that the compositor knows about (including overlays) to a 'screenshots' folder in the SteamVR runtime root. */
		virtual void CompositorDumpImages();

		/** Let an app know it should be rendering with low resources. */
		virtual bool ShouldAppRenderWithLowResources();

		/** Override interleaved reprojection logic to force on. */
		virtual void ForceInterleavedReprojectionOn(bool bOverride);

		/** Force reconnecting to the compositor process. */
		virtual void ForceReconnectProcess();

		/** Temporarily suspends rendering (useful for finer control over scene transitions). */
		virtual void SuspendRendering(bool bSuspend);

		/** Opens a shared D3D11 texture with the undistorted composited image for each eye.  Use ReleaseMirrorTextureD3D11 when finished
		* instead of calling Release on the resource itself. */
		virtual EVRCompositorError GetMirrorTextureD3D11(EVREye eEye, void *pD3D11DeviceOrResource, void **ppD3D11ShaderResourceView);
		virtual void ReleaseMirrorTextureD3D11(void *pD3D11ShaderResourceView);

		/** Access to mirror textures from OpenGL. */
		virtual EVRCompositorError GetMirrorTextureGL(EVREye eEye, glUInt_t *pglTextureId, glSharedTextureHandle_t *pglSharedTextureHandle);
		virtual bool ReleaseSharedGLTexture(glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle);
		virtual void LockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle);
		virtual void UnlockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle);

		/** [Vulkan Only]
		* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
		* null.  The string will be a space separated list of-required instance extensions to enable in VkCreateInstance */
		virtual uint32_t GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char *pchValue, uint32_t unBufferSize);

		/** [Vulkan only]
		* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
		* null.  The string will be a space separated list of required device extensions to enable in VkCreateDevice */
		virtual uint32_t GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T *pPhysicalDevice, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize);
	};
}

using CVRCompositor_020 = CVRCompositor_020_scope::CVRCompositor_020;