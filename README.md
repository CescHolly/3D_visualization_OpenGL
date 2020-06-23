# 3D_visualization_OpenGL_Qt
With the knowledge provided by the IDI subject of FIB at UPC.

Usage:
  1. If you do not have qt, glm, or OpenGL installed you should do the following:
  
    If you are using the openSuse Linux distribution install these packages:
      libqt4-devel
      libQt5Core-devel
      libQt5Designer5                      //not sure if needed
      libQt5DesignerComponents5            //not sure if needed
      libQt5OpenGL-devel
      libQt5OpenGLExtentions-devel-static
      glm-devel

    If you are using a different Linux distribution or Windows there should be equivalent packages.
    
  2. Once everything is installed correctly follow the next steps to start the program:
  
    Open the command line in the 3D_visualiztion_OpenGL_Qt directory and type:
      cd Programa
      qmake-qt5
      make
      ./Visualitzador

If done correctly, a window should open:

<img src="images/3d_visualizer.png" width="600">

If you want to see other 3D models or import your own click on the button "Open .obj file" and a window like the following will let you search the desired file:

<img src="images/load_file_obj.png" width="600">

UI elements:
  1. Camera projection: (Perspective/Orthogonal).
  2. Camera movement:

    2.1 X-axis can be enabled or disabled when rotating the scene, the button next to it with the "camera" symbol centers the camera to the X-axis.
    2.2 Y-axis can be enabled or disabled when rotating the scene, the button next to it with the "camera" symbol centers the camera to the Y-axis.
    2.3 As the Z-axis is not modified when rotating the scene you can do so with a slider, the button next to it with the "camera" symbol centers the camera to the Z-axis.
  3. Light focus color: Three sliders specify the R, G, B values of the color of the light source.
  4. Shader FX:
  
    4.1 Three check-boxes specify which layers of color should be ignored or painted after processing the image (to show the three components separately).
    4.2 The grey scale slider specifies the percentage of saturation and produces the following effect:
    
<img src="images/grey_scale.png" width="600">

  5. Zoom: Slider that expands or shrinks the field of view to achieve a zoom-out / zoom-in effect.

The only features that are not yet shown in the UI and you can use are:
  1. Rotate the scene with mouse left-click and drag.
  2. Rotate the light source around the scene with mouse right-click and drag.
  3. Change to a darker background with 'D' (for dark) and lighter background with 'L' (for light).


