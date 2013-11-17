Test programs for OpenGL engine:
  - texture_test -> multitexturing test with 2 textures (included as files: ./tex1.png, ./tex2.png)
  - model_test -> simple model viewer to test model loading
    usage: ./model_test {old|new_txt|new_bin} model_file
      second argument is the loaded format (DXF or Colobot .mod files)
    requires ./tex folder (or symlink) with Colobot textures
    viewer is controlled from keyboard - the bindings can be found in code
  - transform_test -> simple "walk around" test for world & view transformations
  - light test -> test for lighting
