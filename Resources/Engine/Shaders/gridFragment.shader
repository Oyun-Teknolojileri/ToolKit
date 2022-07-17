<shader>
	<type name = "fragmentShader" />
	<uniform name = "LightData" />
	<uniform name = "CamData" />
	<uniform name = "Model" />
	<uniform name = "InverseTransModel" />
	<uniform name = "GridData" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec2 o_gridPos;
		in vec2 o_cameraGridPos;
		in vec3 o_viewDir;

		struct _GridData
		{
			float cellSize;
			float gridSize;
			vec3 horizontalAxisColor;
			vec3 verticalAxisColor;
		};
		uniform _GridData GridData;


		uniform mat4 InverseTransModel;
		uniform mat4 Model;

		out vec4 fragColor;

		float log10(float x){
			return (1.0 / log(10.0)) * log(x);
		}

		//Shader is based on https://ourmachinery.com/post/borderland-between-rendering-and-editor-part-1/
		void main()
		{
			// UV is grid space coordinate of pixel.
			vec2 uv = o_gridPos;
      // Find screen space derivates in grid space. 
			vec2 dudv = vec2(length(vec2(dFdx(uv.x), dFdy(uv.x))), length(vec2(dFdx(uv.y), dFdy(uv.y))));

			float min_pixels_between_cells = 5.0f;
			float cs = GridData.cellSize;

      // Calc lod-level
      float lod_level = max(0.0f, log10((length(dudv) * min_pixels_between_cells) / cs) + 1.0f);
      float lod_fade = fract(lod_level);

        // Calc cell sizes for lod0, lod1 and lod2
      float lod0_cs = cs * pow(10.0f, floor(lod_level));
      float lod1_cs = lod0_cs * 10.f;
      float lod2_cs = lod1_cs * 10.f;

      // Grid is infinte, so shift uv based on camera position in local tile space
      //uv += o_cameraGridPos;

			
      // Allow each anti-aliased line to cover up to 10 pixels.
      dudv *= 2.0;
      // Offset to pixel center.
      uv = abs(uv) + dudv / 2.0f;


      // Calculate distance to cell line center for each lod and pick max of X,Y to get a coverage alpha value
      // Note: another alternative is to average the x,y values and use that as the covergae alpha (dot(lod0_cross_a, 0.5))
      vec2 lod0_cross_a = 1.f - abs(clamp(mod(uv, lod0_cs) / dudv, 0.0f, 1.0f) * 2.0f - 1.f);
      float lod0_a = max(lod0_cross_a.x, lod0_cross_a.y);
      vec2 lod1_cross_a = 1.f - abs(clamp(mod(uv, lod1_cs) / dudv, 0.0f, 1.0f) * 2.0f - 1.f);
      float lod1_a = max(lod1_cross_a.x, lod1_cross_a.y);
      vec2 lod2_cross_a = 1.f - abs(clamp(mod(uv, lod2_cs) / dudv, 0.0f, 1.0f) * 2.0f - 1.f);
      float lod2_a = max(lod2_cross_a.x, lod2_cross_a.y);

      vec4 thin_color = vec4(vec3(0.203601092), 1.0f);
      vec4 thick_color = vec4(vec3(0.0f), 1.0f);

      // Set XZ axis colors for axis-matching thick lines
      vec2 displaced_grid_pos = o_gridPos;
      bool is_axis_z = lod2_cross_a.x > 0.0f && (-lod1_cs < displaced_grid_pos.x && displaced_grid_pos.x < lod1_cs);
      bool is_axis_x = lod2_cross_a.y > 0.0f && (-lod1_cs < displaced_grid_pos.y && displaced_grid_pos.y < lod1_cs);

			if(is_axis_x){
				thick_color = vec4(GridData.horizontalAxisColor, 1.0f);
			}
			if(is_axis_z){
				thick_color = vec4(GridData.verticalAxisColor, 1.0f);
			}

      // Blend between falloff colors.
			vec4 c = vec4(1.0f);
			if(lod2_a > 0.0f){
				c = thick_color;
			}
			else{
				if(lod1_a > 0.0f){
					c = mix(thick_color, thin_color, lod_fade);
				}
				else{
					c = thin_color;
				}
			}

      // Blend between LOD level alphas and scale with opacity falloff.
			if(lod2_a > 0.0f){
				c.a *= lod2_a;
			}
			else{
				if(lod1_a > 0.0f){
					c.a *= lod1_a;
				}
				else{
					c.a *= lod0_a * (1.0f-lod_fade);
				}
			}
			//c.a *= op;
			fragColor = c;
			//fragColor = vec4(vec3(op), 1.0f);
		}
	-->
	</source>
</shader>