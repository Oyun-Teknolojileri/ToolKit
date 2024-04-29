<shader>
	<type name = "vertexShader" />
    <include name = "skinning.shader" />
    <uniform name = "ProjectViewModel" />
    <uniform name = "InverseTransModel" />
    <uniform name = "Model" />
    <uniform name = "normalMapInUse" />
    <uniform name = "View" />
	<source>
	<!--
      #version 300 es
	    precision highp float;
	    precision lowp int;

      layout(location = 0) in vec3 vPosition;
      layout(location = 1) in vec3 vNormal;
      layout(location = 2) in vec2 vTexture;
      layout(location = 3) in vec3 vBiTan;

      uniform mat4 ProjectViewModel;
      uniform mat4 InverseTransModel;
      uniform mat4 Model;
      uniform mat4 View;

      uniform int normalMapInUse;

      out vec3 v_pos;
      out vec3 v_normal;
      out vec2 v_texture;
      out float v_viewPosDepth;
      out mat3 TBN;

      void main()
      {
         gl_Position = vec4(vPosition, 1.0f);
         if(isSkinned > 0u)
         {
            if (normalMapInUse == 1)
            {
               vec3 B = normalize(vec3(Model * vec4(vBiTan, 0.0)));
               vec3 N = normalize(vec3(Model * vec4(vNormal, 0.0)));

               skin(gl_Position, N, B, gl_Position, N, B);

               vec3 T = normalize(cross(B,N));
               TBN = mat3(T,B,N);
            }
            else
            {
               v_normal = (InverseTransModel * vec4(vNormal, 1.0)).xyz;
               skin(gl_Position, v_normal, gl_Position, v_normal);
            }
         }
         else
         {
			if (normalMapInUse == 1)
			{
               vec3 B = normalize(vec3(Model * vec4(vBiTan, 0.0)));
               vec3 N = normalize(vec3(Model * vec4(vNormal, 0.0)));
               vec3 T = normalize(cross(B,N));
               TBN = mat3(T,B,N);
            }
            else
            {
               v_normal = (InverseTransModel * vec4(vNormal, 1.0)).xyz;
            }
         }

         v_pos = (Model * gl_Position).xyz;
         v_viewPosDepth = (View * Model * gl_Position).z;
         gl_Position = ProjectViewModel * gl_Position;
         v_texture = vTexture;
      }
	-->
	</source>
</shader>