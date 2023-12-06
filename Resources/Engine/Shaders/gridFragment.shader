<shader>
	<type name = "fragmentShader" />
	<include name = "camera.shader" />
	<uniform name = "Model" />
	<uniform name = "InverseTransModel" />
	<source>
	<!--
		// This algorithm works as:
		// Find the best 2-line 1-area combination for current distance between plane-camera
		// For each pixel, calculate which line current pixel is in by
		//		finding uv derivatives of the grid mesh.
		//	a) lod2_a > 0.0f means thick line.
		//	b) lod1_a > 0.0f means medium line.
		//	c) remaining pixels are in Remaining Space
		//	Note: We don't use "no line" as "Remaining Space" because 3D viewport uses "Remaining Space" to show cells there
		// Difference between Viewport Types (2D, 3D) is handling these line-areas as different colors and alphas.
		//	2D:
		//	a) Don't use user-provided cell size, divide it by 10.
		//			So smallest cell (user-provided) lines changes execution path
		//			Previously they're using lod1_a (for 10th cells) & empty area (for smaller cells & empty area).
		//			Now they're using lod2_a (for 10th lines), lod1_a (for smaller cells) & empty area.
		//	b) Remove thin lines with alpha 0.0 & use medium lines as thin lines with alpha 1.0
		//			Color blending already gives enough blending for the current color
		//	3D:
		//  a) Calculate pixel's distance to nearest cell LoD's lines.
		//	b) Use these distances to set alpha.
		//  NOTE: In Remaining Space; use inverse distance to get
		//					both empty space & very very small cells if precision is enough.


		// This shader
		#version 300 es
		precision highp float;
		precision mediump int;

		in vec2 o_gridPos;
		in vec2 o_cameraGridPos;
		in vec3 o_viewDir;
		in vec3 v_pos;

		const float gridCullDistance = 10000.0f;
		struct _GridData
		{
			float cellSize;
			float lineMaxPixelCount;
			vec3 horizontalAxisColor;
			vec3 verticalAxisColor;
			uint is2DViewport; //!-< True: Viewport is 2D. False: Viewport is 3D.
			float cullDistance;	//!-< Not used for now, gridCullDistance constant above is used for now
		};
		uniform _GridData GridData;
		
		uniform mat4 ProjectViewModel;
		uniform mat4 InverseTransModel;
		uniform mat4 Model;

		out vec4 fragColor;

		float log10(float x){
			return (1.0 / log(10.0)) * log(x);
		}

		void main()
		{
			vec3 fragToCamera = CamData.pos - v_pos;

			// UV is grid space coordinate of pixel.
			vec2 uv = o_gridPos;
			// Find screen space derivates in grid space.
			vec2 dudv = vec2(length(vec2(dFdx(uv.x), dFdy(uv.x))), length(vec2(dFdx(uv.y), dFdy(uv.y))));

			float min_pixels_between_cells = 1.0f;
			float cs = GridData.cellSize / ((GridData.is2DViewport > 0u) ? 10.0f : 1.0f);

			// Calc lod-level
			float lod_level = max(0.0f, log10((length(dudv) * min_pixels_between_cells) / cs) + 1.0f);
			float lod_fade = fract(lod_level);

			// Calc cell sizes for lod0, lod1 and lod2
			float lod0_cs = cs * pow(10.0f, floor(lod_level));
			float lod1_cs = lod0_cs * 10.f;
			float lod2_cs = lod1_cs * 10.f;


			// Allow each anti-aliased line to cover up to N pixels.
			dudv *= GridData.lineMaxPixelCount;
			// Offset to pixel center
			uv = abs(uv) + dudv / GridData.lineMaxPixelCount;

			// For each cell LoD: calculate distance to nearest cell's line center
			//		and pick max of X,Y to get a coverage alpha value
			vec2 lod0_cross_a = 1.f - abs(clamp(mod(uv, lod0_cs) / dudv, 0.0f, 1.0f) * 2.0f - 1.f);
			float lod0_a = max(lod0_cross_a.x, lod0_cross_a.y);
			vec2 lod1_cross_a = 1.f - abs(clamp(mod(uv, lod1_cs) / dudv, 0.0f, 1.0f) * 2.0f - 1.f);
			float lod1_a = max(lod1_cross_a.x, lod1_cross_a.y);
			vec2 lod2_cross_a = 1.f - abs(clamp(mod(uv, lod2_cs) / dudv, 0.0f, 1.0f) * 2.0f - 1.f);
			float lod2_a = max(lod2_cross_a.x, lod2_cross_a.y);

			vec4 thin_color = vec4(vec3(0.0736f), 1.0f);
			vec4 thick_color = vec4(vec3(0.1536f), 1.0f);

			// Set XZ axis colors for axis-matching thick lines
			bool is_axis_z = lod2_cross_a.x > 0.0f && (-lod1_cs < o_gridPos.x && o_gridPos.x < lod1_cs);
			bool is_axis_x = lod2_cross_a.y > 0.0f && (-lod1_cs < o_gridPos.y && o_gridPos.y < lod1_cs);

			if(is_axis_x){
				thick_color = vec4(GridData.horizontalAxisColor, 1.0f);
			}
			if(is_axis_z){
				thick_color = vec4(GridData.verticalAxisColor, 1.0f);
			}
			
			// Blend between falloff colors.
			vec4 c = vec4(1.0f);
			if(lod2_a > 0.0f){
				// Thick line (thick ones, main zero-axises too) for current distance
				c = thick_color;
				c.a *= lod2_a;
			}
			else{
				if(lod1_a > 0.0f){
					// Medium line except 10ths (they're thick) for current distance
					c = mix(thick_color, thin_color, lod_fade);
					c.a *= lod1_a;
				}
				else{
					// Remaining Space
					// Smooth transition achieved by lod_fade lerp
					c = thin_color;
					c.a *= lod0_a * (1.0f-lod_fade);
				}
			}
			if (GridData.is2DViewport == 0u)
			{
				// Attenuation
				float constant = 1.0;
				float linear = 0.009;
				float quadratic = 0.005;
				float distance = length(fragToCamera);
				float attenuation = 1.0 /
				(constant + linear * distance + quadratic * (distance * distance));
				c.a *= attenuation;

				// Cull if dudv is too much to stabilly render a line
				// This is happening in gracing angles and very very far fragments in 3D
				if(length(dudv) > 2.0){discard;}
			}
			
			if (clamp(gridCullDistance - length(fragToCamera), -1.0, 1.0) < 0.0f)
			{
				discard;
			}
			fragColor = c;
		}
	-->
	</source>
</shader>