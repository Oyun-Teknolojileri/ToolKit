<shader>
	<type name = "vertexShader" />
   <include name = "skinning.shader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<uniform name = "Model" />
   <uniform name = "normalMapInUse" />
	<source>
	<!--
      #version 300 es

      layout(location = 0) in vec3 vPosition;
      layout(location = 1) in vec3 vNormal;
      layout(location = 2) in vec2 vTexture;
      layout(location = 3) in vec3 vBiTan;

      uniform mat4 ProjectViewModel;
      uniform mat4 InverseTransModel;
      uniform mat4 Model;
      uniform uint isSkinned;

      uniform int normalMapInUse;

      out vec3 v_pos;
      out vec3 v_normal;
      out vec2 v_texture;
      out mat3 TBN;

      void main()
      {
         gl_Position = vec4(vPosition, 1.0f);
         if(isSkinned > 0u){
            gl_Position = skin(gl_Position);
         }
         
         v_pos = (Model * gl_Position).xyz;
         gl_Position = ProjectViewModel * gl_Position;
         v_texture = vTexture;

         if (normalMapInUse == 1)
         {
            vec3 B = skinNormal(normalize(vec3(Model * vec4(vBiTan, 0.0))));
            vec3 N = skinNormal(normalize(vec3(Model * vec4(vNormal, 0.0))));
            vec3 T = normalize(cross(B,N));
            TBN = mat3(T,B,N);
         }
         else
         {
            v_normal = skinNormal((InverseTransModel * vec4(vNormal, 1.0)).xyz);
         }
      }
	-->
	</source>
</shader>