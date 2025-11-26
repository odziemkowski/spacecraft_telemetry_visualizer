# spacecraft_telemetry_visualizer

## Troubleshooting: orientation marker / triad and GL errors

If you added an orientation triad (axes) and are seeing frequent OpenGL errors like:

```
GL_INVALID_FRAMEBUFFER_OPERATION in glBlitFramebuffer(incomplete draw/read buffers)
Error glBlitFramebuffer1 OpenGL errors detected
0 : (1286) Invalid framebuffer operation
```

This is usually caused by a mismatch between the QVTK widget's internal framebuffer or interactor and a separately-created vtkRenderWindowInteractor or a render window with different multi-sample settings.

What we changed in this repo to address it:

- Use the QVTK widget's built-in interactor (via `vtkWidget->interactor()`) for the orientation marker widget instead of creating a separate `vtkRenderWindowInteractor`.
- Ensure multisampling is disabled on the `vtkGenericOpenGLRenderWindow` before attaching it to the `QVTKOpenGLNativeWidget` (call `SetMultiSamples(0)` early).

How to test locally

1. Rebuild and run the application on a machine with Qt and VTK installed.
2. Open the application and verify that the orientation triad is visible.
3. Watch the application logs for the GL errors above — they should no longer appear when the triad/marker is enabled.

If the problem persists, please capture a short terminal log (stdout/stderr) from running the app and attach it when reporting back — that will help us look deeper.
