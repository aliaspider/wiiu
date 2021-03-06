/* coreinit */
FIMPORT_BEGIN(coreinit)
FIMPORT(OSFatal)
FIMPORT(OSDynLoad_Acquire)
FIMPORT(OSDynLoad_FindExport)
FIMPORT(OSDynLoad_Release)
FIMPORT(OSDynLoad_GetTLSAllocator)
FIMPORT(OSDynLoad_SetTLSAllocator)
FIMPORT(OSSetExceptionCallback)
FIMPORT(OSSetExceptionCallbackEx)
FIMPORT(OSSetDABR)
FIMPORT(OSSetIABR)
FIMPORT(OSSavesDone_ReadyToRelease)
FIMPORT(OSInitMutex)
FIMPORT(OSInitMutexEx)
FIMPORT(OSLockMutex)
FIMPORT(OSTryLockMutex)
FIMPORT(OSUnlockMutex)
FIMPORT(OSInitCond)
FIMPORT(OSWaitCond)
FIMPORT(OSSignalCond)
FIMPORT(OSInitSpinLock)
FIMPORT(OSUninterruptibleSpinLock_Acquire)
FIMPORT(OSUninterruptibleSpinLock_Release)
FIMPORT(OSFastMutex_Init)
FIMPORT(OSFastMutex_Lock)
FIMPORT(OSFastMutex_Unlock)
FIMPORT(OSSleepTicks)
FIMPORT(OSGetTitleID)
FIMPORT(OSIsThreadTerminated)
FIMPORT(OSSetThreadPriority)
FIMPORT(OSSetThreadName)
FIMPORT(OSCreateThread)
FIMPORT(OSSetThreadCleanupCallback)
FIMPORT(OSResumeThread)
FIMPORT(OSIsThreadSuspended)
FIMPORT(OSSuspendThread)
FIMPORT(OSGetCurrentThread)
FIMPORT(OSGetThreadName)
FIMPORT(OSGetThreadSpecific)
FIMPORT(OSSetThreadSpecific)
FIMPORT(OSSetThreadDeallocator)
FIMPORT(OSSetThreadRunQuantum)
FIMPORT(OSExitThread)
FIMPORT(OSJoinThread)
FIMPORT(OSYieldThread)
FIMPORT(OSDetachThread)
FIMPORT(OSGetDefaultThread)
FIMPORT(OSContinueThread)
FIMPORT(OSGetCoreId)
FIMPORT(OSIsMainCore)
FIMPORT(PPCSetFpIEEEMode)
FIMPORT(PPCSetFpNonIEEEMode)
FIMPORT(OSGetSystemTime)
FIMPORT(OSGetSystemTick)
FIMPORT(OSGetTime)
FIMPORT(OSGetSymbolName)
FIMPORT(OSGetSharedData)
FIMPORT(OSBlockMove)
FIMPORT(OSBlockSet)
FIMPORT(OSEffectiveToPhysical)
FIMPORT(OSAllocFromSystem)
FIMPORT(OSFreeToSystem)
FIMPORT(OSAllocVirtAddr)
FIMPORT(OSFreeVirtAddr)
FIMPORT(OSQueryVirtAddr)
FIMPORT(OSGetMapVirtAddrRange)
FIMPORT(OSGetDataPhysAddrRange)
FIMPORT(OSGetAvailPhysAddrRange)
FIMPORT(OSMapMemory)
FIMPORT(OSUnmapMemory)
FIMPORT(OSInitSemaphore)
FIMPORT(OSInitSemaphoreEx)
FIMPORT(OSGetSemaphoreCount)
FIMPORT(OSSignalSemaphore)
FIMPORT(OSWaitSemaphore)
FIMPORT(OSTryWaitSemaphore)
FIMPORT(OSTestAndSetAtomic)
FIMPORT(OSCompareAndSwapAtomic)
FIMPORT(OSCompareAndSwapAtomicEx)
FIMPORT(OSCreateAlarm)
FIMPORT(OSSetAlarmUserData)
FIMPORT(OSGetAlarmUserData)
FIMPORT(OSSetAlarm)
FIMPORT(OSCancelAlarm)
FIMPORT(OSGetSystemInfo)
FIMPORT(OSScreenInit)
FIMPORT(OSScreenGetBufferSizeEx)
FIMPORT(OSScreenSetBufferEx)
FIMPORT(OSScreenClearBufferEx)
FIMPORT(OSScreenFlipBuffersEx)
FIMPORT(OSScreenPutFontEx)
FIMPORT(OSScreenPutPixelEx)
FIMPORT(OSScreenEnableEx)
FIMPORT(exit)
FIMPORT(_Exit)
FIMPORT(OSConsoleWrite)
FIMPORT(OSReport)
FIMPORT(__os_snprintf)
FIMPORT(DisassemblePPCRange)
FIMPORT(DisassemblePPCOpcode)
FIMPORT(DCInvalidateRange)
FIMPORT(DCFlushRange)
FIMPORT(DCFlushRangeNoSync)
FIMPORT(DCStoreRange)
FIMPORT(DCStoreRangeNoSync)
FIMPORT(ICInvalidateRange)
FIMPORT(__gh_errno_ptr)
FIMPORT(MEMGetBaseHeapHandle)
FIMPORT(MEMCreateExpHeapEx)
FIMPORT(MEMDestroyExpHeap)
FIMPORT(MEMAllocFromExpHeapEx)
FIMPORT(MEMFreeToExpHeap)
FIMPORT(MEMGetTotalFreeSizeForExpHeap)
FIMPORT(MEMGetSizeForMBlockExpHeap)
FIMPORT(MEMAllocFromFrmHeapEx)
FIMPORT(MEMFreeToFrmHeap)
FIMPORT(MEMGetAllocatableSizeForFrmHeapEx)
FIMPORT(FSInit)
FIMPORT(FSShutdown)
FIMPORT(FSAddClient)
FIMPORT(FSAddClientEx)
FIMPORT(FSDelClient)
FIMPORT(FSInitCmdBlock)
FIMPORT(FSSetCmdPriority)
FIMPORT(FSChangeDir)
FIMPORT(FSGetFreeSpaceSize)
FIMPORT(FSGetStat)
FIMPORT(FSRemove)
FIMPORT(FSOpenFile)
FIMPORT(FSCloseFile)
FIMPORT(FSOpenDir)
FIMPORT(FSMakeDir)
FIMPORT(FSReadDir)
FIMPORT(FSRewindDir)
FIMPORT(FSCloseDir)
FIMPORT(FSGetStatFile)
FIMPORT(FSReadFile)
FIMPORT(FSWriteFile)
FIMPORT(FSSetPosFile)
FIMPORT(FSFlushFile)
FIMPORT(FSTruncateFile)
FIMPORT(FSRename)
FIMPORT(FSGetMountSource)
FIMPORT(FSMount)
FIMPORT(FSUnmount)
FIMPORT(IOS_Open)
FIMPORT(IOS_Close)
FIMPORT(IOS_Ioctl)
FIMPORT(IOS_IoctlAsync)
FIMPORT(IMIsAPDEnabled)
FIMPORT(IMIsDimEnabled)
FIMPORT(IMEnableAPD)
FIMPORT(IMEnableDim)
FIMPORT(IMDisableAPD)
FIMPORT(IMDisableDim)
FIMPORT(__tls_get_addr)
/* nsysnet */
FIMPORT_BEGIN(nsysnet)
FIMPORT(socket_lib_init)
FIMPORT(getaddrinfo)
FIMPORT(freeaddrinfo)
FIMPORT(getnameinfo)
FIMPORT(inet_ntoa)
FIMPORT(inet_ntop)
FIMPORT(inet_aton)
FIMPORT(inet_pton)
FIMPORT(ntohl)
FIMPORT(ntohs)
FIMPORT(htonl)
FIMPORT(htons)
FIMPORT(accept)
FIMPORT(bind)
FIMPORT(socketclose)
FIMPORT(connect)
FIMPORT(getpeername)
FIMPORT(getsockname)
FIMPORT(getsockopt)
FIMPORT(listen)
FIMPORT(recv)
FIMPORT(recvfrom)
FIMPORT(send)
FIMPORT(sendto)
FIMPORT(setsockopt)
FIMPORT(shutdown)
FIMPORT(socket)
FIMPORT(select)
FIMPORT(socketlasterr)
FIMPORT(gethostbyname)
FIMPORT(gai_strerror)
/* gx2 */
FIMPORT_BEGIN(gx2)
FIMPORT(GX2Invalidate)
FIMPORT(GX2Init)
FIMPORT(GX2GetSystemTVScanMode)
FIMPORT(GX2CalcTVSize)
FIMPORT(GX2SetTVBuffer)
FIMPORT(GX2CalcDRCSize)
FIMPORT(GX2SetDRCBuffer)
FIMPORT(GX2CalcSurfaceSizeAndAlignment)
FIMPORT(GX2CopySurface)
FIMPORT(GX2CopySurfaceEx)
FIMPORT(GX2InitColorBufferRegs)
FIMPORT(GX2InitDepthBufferRegs)
FIMPORT(GX2SetupContextStateEx)
FIMPORT(GX2SetContextState)
FIMPORT(GX2SetColorBuffer)
FIMPORT(GX2SetDepthBuffer)
FIMPORT(GX2SetViewport)
FIMPORT(GX2SetViewportReg)
FIMPORT(GX2SetScissor)
FIMPORT(GX2GetScissorReg)
FIMPORT(GX2SetScissorReg)
FIMPORT(GX2SetDepthOnlyControl)
FIMPORT(GX2InitDepthStencilControlReg)
FIMPORT(GX2InitStencilMaskReg)
FIMPORT(GX2SetColorControl)
FIMPORT(GX2InitColorControlReg)
FIMPORT(GX2InitTargetChannelMasksReg)
FIMPORT(GX2SetBlendControl)
FIMPORT(GX2InitBlendControlReg)
FIMPORT(GX2SetBlendControlReg)
FIMPORT(GX2SetColorControlReg)
FIMPORT(GX2SetTargetChannelMasksReg)
FIMPORT(GX2SetDepthStencilControlReg)
FIMPORT(GX2SetBlendConstantColor)
FIMPORT(GX2SetBlendConstantColorReg)
FIMPORT(GX2SetCullOnlyControl)
FIMPORT(GX2CalcFetchShaderSizeEx)
FIMPORT(GX2InitFetchShaderEx)
FIMPORT(GX2SetFetchShader)
FIMPORT(GX2SetVertexShader)
FIMPORT(GX2SetPixelShader)
FIMPORT(GX2SetGeometryShader)
FIMPORT(GX2SetGeometryUniformBlock)
FIMPORT(GX2SetVertexUniformBlock)
FIMPORT(GX2SetPixelUniformBlock)
FIMPORT(GX2CalcGeometryShaderInputRingBufferSize)
FIMPORT(GX2CalcGeometryShaderOutputRingBufferSize)
FIMPORT(GX2SetGeometryShaderInputRingBuffer)
FIMPORT(GX2SetGeometryShaderOutputRingBuffer)
FIMPORT(GX2SetShaderModeEx)
FIMPORT(GX2SetAttribBuffer)
FIMPORT(GX2InitTextureRegs)
FIMPORT(GX2InitSampler)
FIMPORT(GX2InitSamplerBorderType)
FIMPORT(GX2InitSamplerClamping)
FIMPORT(GX2InitSamplerDepthCompare)
FIMPORT(GX2InitSamplerLOD)
FIMPORT(GX2InitSamplerXYFilter)
FIMPORT(GX2InitSamplerZMFilter)
FIMPORT(GX2SetPixelTexture)
FIMPORT(GX2SetPixelSampler)
FIMPORT(GX2SetVertexTexture)
FIMPORT(GX2SetStencilMask)
FIMPORT(GX2SetStencilMaskReg)
FIMPORT(GX2ClearColor)
FIMPORT(GX2ClearBuffersEx)
FIMPORT(GX2ClearDepthStencilEx)
FIMPORT(GX2CopyColorBufferToScanBuffer)
FIMPORT(GX2SwapScanBuffers)
FIMPORT(GX2Flush)
FIMPORT(GX2WaitForVsync)
FIMPORT(GX2SetTVEnable)
FIMPORT(GX2SetDRCEnable)
FIMPORT(GX2SetSwapInterval)
FIMPORT(GX2DrawDone)
FIMPORT(GX2Shutdown)
FIMPORT(GX2DrawEx)
FIMPORT(GX2DrawIndexedEx)
FIMPORT(GX2DrawIndexedImmediateEx)
FIMPORT(GX2WaitForFlip)
FIMPORT(GX2GetSwapStatus)
FIMPORT(GX2ResetGPU)
FIMPORT(GX2AllocateTilingApertureEx)
FIMPORT(GX2FreeTilingAperture)
/* proc_ui */
FIMPORT_BEGIN(proc_ui)
FIMPORT(ProcUIInit)
FIMPORT(ProcUIShutdown)
/* sndcore2 */
FIMPORT_BEGIN(sndcore2)
FIMPORT(AXInitWithParams)
FIMPORT(AXQuit)
FIMPORT(AXRegisterFrameCallback)
FIMPORT(AXAcquireMultiVoice)
FIMPORT(AXSetMultiVoiceDeviceMix)
FIMPORT(AXSetMultiVoiceOffsets)
FIMPORT(AXSetMultiVoiceCurrentOffset)
FIMPORT(AXSetMultiVoiceState)
FIMPORT(AXSetMultiVoiceVe)
FIMPORT(AXSetMultiVoiceSrcType)
FIMPORT(AXSetMultiVoiceSrcRatio)
FIMPORT(AXIsMultiVoiceRunning)
FIMPORT(AXFreeMultiVoice)
FIMPORT(AXGetVoiceOffsets)
/* sysapp */
FIMPORT_BEGIN(sysapp)
FIMPORT(SYSRelaunchTitle)
FIMPORT(SYSLaunchMenu)
/* vpad */
FIMPORT_BEGIN(vpad)
FIMPORT(VPADInit)
FIMPORT(VPADRead)
FIMPORT(VPADSetSamplingCallback)
FIMPORT(VPADGetTPCalibratedPoint)
FIMPORT(VPADGetTPCalibratedPointEx)
/* padscore */
FIMPORT_BEGIN(padscore)
FIMPORT(KPADInit)
FIMPORT(WPADProbe)
FIMPORT(WPADSetDataFormat)
FIMPORT(WPADEnableURCC)
FIMPORT(WPADEnableWiiRemote)
FIMPORT(WPADRead)
FIMPORT(KPADRead)
FIMPORT(KPADReadEx)
/* nsyskbd */
FIMPORT_BEGIN(nsyskbd)
FIMPORT(KBDSetup)
FIMPORT(KBDTeardown)
