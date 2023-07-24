<shader>
	  <type name = "vertexShader" />
	  <include name = "skinning.shader" />
  	<uniform name = "ProjectViewModel" />
	  <uniform name = "Model" />
	  <uniform name = "View" />
	  <uniform name = "InverseTransModel" />
	<source>
	<!--
      #version 300 es
      precision highp int;

      layout(location = 0) in vec3 vPosition;
      layout(location = 1) in vec3 vNormal;
      layout(location = 2) in vec2 vTexture;
      layout(location = 3) in vec3 vBiTan;

      uniform mat4 ProjectViewModel;
      uniform mat4 InverseTransModel;
      uniform mat4 Model;
      uniform mat4 View;
      uniform uint isSkinned;

      // out vec3 v_pos;
      out vec3 v_linearDepth;
      out vec3 v_normal;

      void main()
      {
         gl_Position   = vec4(vPosition, 1.0f);       
         vec3 v_pos    = (Model * gl_Position).xyz;
         v_linearDepth = (View * vec4(v_pos, 1.0)).xyz;

         v_normal = (InverseTransModel * vec4(vNormal, 1.0)).xyz;
         if (isSkinned > 0u)
         {
           v_normal = skinNormal(v_normal);
         }

         gl_Position   = ProjectViewModel * gl_Position;
      }
	-->
	</source>
</shader>