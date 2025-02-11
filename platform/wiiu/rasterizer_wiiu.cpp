/*************************************************************************/
/*  rasterizer_iphone.cpp                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifdef __WIIU__

#include "rasterizer_wiiu.h"
#include "globals.h"
#include "os/os.h"
#include "os_wiiu.h"
#include <stdio.h>

#define print_line

_FORCE_INLINE_ static void _gl_load_transform(const Transform& tr) {

	GLfloat matrix[16]={ /* build a 16x16 matrix */
		tr.basis.elements[0][0],
		tr.basis.elements[1][0],
		tr.basis.elements[2][0],
		0,
		tr.basis.elements[0][1],
		tr.basis.elements[1][1],
		tr.basis.elements[2][1],
		0,
		tr.basis.elements[0][2],
		tr.basis.elements[1][2],
		tr.basis.elements[2][2],
		0,
		tr.origin.x,
		tr.origin.y,
		tr.origin.z,
		1
	};

	// glLoadMatrixf(matrix);
};


_FORCE_INLINE_ static void _gl_mult_transform(const Transform& tr) {

	GLfloat matrix[16]={ /* build a 16x16 matrix */
		tr.basis.elements[0][0],
		tr.basis.elements[1][0],
		tr.basis.elements[2][0],
		0,
		tr.basis.elements[0][1],
		tr.basis.elements[1][1],
		tr.basis.elements[2][1],
		0,
		tr.basis.elements[0][2],
		tr.basis.elements[1][2],
		tr.basis.elements[2][2],
		0,
		tr.origin.x,
		tr.origin.y,
		tr.origin.z,
		1
	};

	// glMultMatrixf(matrix);
};

_FORCE_INLINE_ static void _gl_mult_transform(const Matrix32& tr) {


	// glMultMatrixf(matrix);
};


RasterizerWIIU::FX::FX() {

	bgcolor_active=false;
	bgcolor=Color(0,1,0,1);

	skybox_active=false;

	glow_active=false;
	glow_passes=4;
	glow_attenuation=0.7;
	glow_bloom=0.0;

	antialias_active=true;
	antialias_tolerance=15;

	ssao_active=true;
	ssao_attenuation=0.7;
	ssao_radius=0.18;
	ssao_max_distance=1.0;
	ssao_range_min=0.25;
	ssao_range_max=0.48;
	ssao_only=false;


	fog_active=false;
	fog_near=5;
	fog_far=100;
	fog_attenuation=1.0;
	fog_color_near=Color(1,1,1,1);
	fog_color_far=Color(1,1,1,1);
	fog_bg=false;

	toon_active=false;
	toon_treshold=0.4;
	toon_soft=0.001;

	edge_active=false;
	edge_color=Color(0,0,0,1);
	edge_size=1.0;

}

static GX2RBuffer tex_coord_buffer;
static GX2RBuffer position_buffer;

static const GX2PrimitiveMode prim_type[]={GX2_PRIMITIVE_MODE_POINTS,GX2_PRIMITIVE_MODE_LINES,GX2_PRIMITIVE_MODE_TRIANGLES,GX2_PRIMITIVE_MODE_TRIANGLE_FAN};



static void _draw_primitive(int p_points, float* p_vertices, const Vector3 *p_normals, const Color* p_colors, float* p_uvs,const Plane *p_tangents=NULL,int p_instanced=1) {
	void *buffer1 = NULL;
	void *buffer = NULL;
	void *buffer2 = NULL;

	float* uvs = NULL;

	OSReport("henlo\n");


	ERR_FAIL_COND(!p_vertices);
	ERR_FAIL_COND(p_points <1 || p_points>4);

	GX2PrimitiveMode type = prim_type[p_points - 1];

	const float s_positionData[8] = {
		p_vertices[0],
		p_vertices[1],
		// p_vertices[2],
		p_vertices[3],
		p_vertices[4],
		// p_vertices[5],
		p_vertices[6],
		p_vertices[7],
		// p_vertices[8],
		p_vertices[9],
		p_vertices[10],
		// p_vertices[11],
	};

    const float s_texCoords[8] = {
		p_uvs[0],
		p_uvs[1],
		p_uvs[2],
		p_uvs[3],
		p_uvs[4],
		p_uvs[5],
		p_uvs[6],
		p_uvs[7]
	};

	// GX2RBuffer position_buffer;

	position_buffer.flags =	(GX2RResourceFlags)(GX2R_RESOURCE_BIND_VERTEX_BUFFER | GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ);

	position_buffer.elemSize = 2 * sizeof(float);
	position_buffer.elemCount = 4;
	position_buffer.buffer = NULL;

	GX2RCreateBuffer(&position_buffer);
	buffer = GX2RLockBufferEx(&position_buffer, (GX2RResourceFlags)0);
	memcpy(buffer, p_vertices, position_buffer.elemSize * position_buffer.elemCount);
	GX2RUnlockBufferEx(&position_buffer, (GX2RResourceFlags)0);

	GX2RSetAttributeBuffer(&position_buffer, 0, position_buffer.elemSize, 0);

	// GX2RSetAttributeBuffer(&position_buffer, 0, position_buffer.elemSize, 0);

	// float* verts = (float*)malloc(sizeof(p_vertices));

	// memcpy(verts, (const void*)p_vertices, sizeof(p_vertices));



	//if (!p_colors) {
	//	glColor4f(1, 1, 1, 1);
	//};

	// glEnableClientState(GL_VERTEX_ARRAY);
	// glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)p_vertices);

	if (p_normals) {

			// glEnableClientState(GL_NORMAL_ARRAY);
			// glNormalPointer(GL_FLOAT, 0, (GLvoid*)p_normals);
	};

	if (p_colors) {/*
		GX2RBuffer color_buffer;
		color_buffer.flags = (GX2RResourceFlags)(GX2R_RESOURCE_BIND_COLOR_BUFFER | GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ | GX2R_RESOURCE_USAGE_GPU_WRITE);

		color_buffer.elemSize = 4 * p_points;
		color_buffer.elemCount = p_points;
		color_buffer.buffer = NULL;

		GX2RCreateBuffer(&color_buffer);
		buffer2 = GX2RLockBufferEx(&color_buffer, (GX2RResourceFlags)0);
		memcpy(buffer2, p_colors, color_buffer.elemSize * color_buffer.elemCount);
		GX2RUnlockBufferEx(&color_buffer, (GX2RResourceFlags)0);

		GX2RSetAttributeBuffer(&color_buffer, 1, color_buffer.elemSize, 0);*/
	};

	if (p_uvs) {

		// uvs = (float*)malloc(sizeof(p_uvs));

		// memcpy(uvs, (const void*)p_uvs, sizeof(p_uvs));

		// GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER), p_uvs, 4 * sizeof(float));


		// GX2SetAttribBuffer(1, 4 * sizeof(*p_uvs), sizeof(*p_uvs), p_uvs);


		// GX2RBuffer tex_coord_buffer;

		tex_coord_buffer.flags = (GX2RResourceFlags)(GX2R_RESOURCE_BIND_VERTEX_BUFFER | GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ);

		tex_coord_buffer.elemSize = 2 * sizeof(float);
		tex_coord_buffer.elemCount = 4;
		tex_coord_buffer.buffer = NULL;

		GX2RCreateBuffer(&tex_coord_buffer);
		buffer1 = GX2RLockBufferEx(&tex_coord_buffer, (GX2RResourceFlags)0);
		memcpy(buffer1, p_uvs, tex_coord_buffer.elemSize * tex_coord_buffer.elemCount);
		GX2RUnlockBufferEx(&tex_coord_buffer, (GX2RResourceFlags)0);

		// GX2RSetAttributeBuffer(1, 4 * sizeof(float), sizeof(float), p_uvs);

		GX2RSetAttributeBuffer(&tex_coord_buffer, 1, tex_coord_buffer.elemSize, 0);

		// GX2SetAttribBuffer(1, 4 * sizeof(*p_uvs), sizeof(*p_uvs), p_uvs);

// 			glClientActiveTexture(GL_TEXTURE0);
			// glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			// glTexCoordPointer(3, GL_FLOAT, 0, p_uvs);
	}


	// glDrawArrays( type, 0, p_points);

	// glDisableClientState(GL_VERTEX_ARRAY);
	// glDisableClientState(GL_NORMAL_ARRAY);
	// glDisableClientState(GL_COLOR_ARRAY);
	// glDisableClientState(GL_TEXTURE_COORD_ARRAY);
/*
	for(int i = 0; i < 3 * 4; i++)
		OSReport("%f\n", p_vertices[i]);*/

	// GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER), p_vertices, sizeof(float));

	// GX2RSetAttributeBuffer(0, 4 * sizeof(float), sizeof(float), p_vertices);



/*
	free(verts);

	if(uvs)
		free(uvs);*/
};

static void _draw_gui_primitive(int p_points, const Vector2 *p_vertices, const Color* p_colors, const Vector2 *p_uvs) {

	ERR_FAIL_COND(!p_vertices);
	ERR_FAIL_COND(p_points <1 || p_points>4);

	GLenum type = prim_type[p_points - 1];


	//if (!p_colors) {
	//	glColor4f(1, 1, 1, 1);
	//};

	// glEnableClientState(GL_VERTEX_ARRAY);
	// glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)p_vertices);


	if (p_colors) {
			// glEnableClientState(GL_COLOR_ARRAY);
			// glColorPointer(4,GL_FLOAT, 0, p_colors);
	};

	if (p_uvs) {

// 			glClientActiveTexture(GL_TEXTURE0);
			// glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			// glTexCoordPointer(3, GL_FLOAT, 0, p_uvs);
	};

	// glDrawArrays( type, 0, p_points);

	// glDisableClientState(GL_VERTEX_ARRAY);
	// glDisableClientState(GL_NORMAL_ARRAY);
	// glDisableClientState(GL_COLOR_ARRAY);
	// glDisableClientState(GL_TEXTURE_COORD_ARRAY);
};

void RasterizerWIIU::_draw_tex_bg() {

	// glDepthMask(GL_TRUE);
	// glEnable(GL_DEPTH_TEST);
	// glDisable(GL_CULL_FACE);
	// glDisable(GL_BLEND);
	// glColorMask(1, 1, 1, 1);

	RID texture;

	if (current_env->bg_mode == VS::ENV_BG_TEXTURE) {
		texture = current_env->bg_param[VS::ENV_BG_PARAM_TEXTURE];
	} else {
		texture = current_env->bg_param[VS::ENV_BG_PARAM_CUBEMAP];
	}

	if (!texture_owner.owns(texture)) {
		return;
	}

	Texture *t = texture_owner.get(texture);

// 	glActiveTexture(GL_TEXTURE0);
	// glBindTexture(t->target, t->tex_id);


	float nrg = float(current_env->bg_param[VS::ENV_BG_PARAM_ENERGY]);
	if (current_env->fx_enabled[VS::ENV_FX_HDR])
		nrg *= 0.25; //go down a quarter for hdr


	float flip_sign = (current_env->bg_mode == VS::ENV_BG_TEXTURE) ? -1 : 1;

	Vector3 vertices[4] = {
		Vector3(-1, -1 * flip_sign, 1),
		Vector3(1, -1 * flip_sign, 1),
		Vector3(1, 1 * flip_sign, 1),
		Vector3(-1, 1 * flip_sign, 1)
	};

	Vector3 src_uv[4] = {
		Vector3(0, 1, 0),
		Vector3(1, 1, 0),
		Vector3(1, 0, 0),
		Vector3(0, 0, 0)
	};

	if (current_env->bg_mode == VS::ENV_BG_TEXTURE) {

		//regular texture
		//adjust aspect

		float aspect_t = t->width / float(t->height);
		float aspect_v = viewport.width / float(viewport.height);

		if (aspect_v > aspect_t) {
			//wider than texture
			for (int i = 0; i < 4; i++) {
				src_uv[i].y = (src_uv[i].y - 0.5) * (aspect_t / aspect_v) + 0.5;
			}

		} else {
			//narrower than texture
			for (int i = 0; i < 4; i++) {
				src_uv[i].x = (src_uv[i].x - 0.5) * (aspect_v / aspect_t) + 0.5;
			}
		}

		float scale = current_env->bg_param[VS::ENV_BG_PARAM_SCALE];
		for (int i = 0; i < 4; i++) {

			src_uv[i].x *= scale;
			src_uv[i].y *= scale;
		}
	} else {

		//skybox uv vectors
		float vw, vh, zn;
		camera_projection.get_viewport_size(vw, vh);
		zn = camera_projection.get_z_near();

		float scale = current_env->bg_param[VS::ENV_BG_PARAM_SCALE];

		for (int i = 0; i < 4; i++) {

			Vector3 uv = src_uv[i];
			uv.x = (uv.x * 2.0 - 1.0) * vw * scale;
			uv.y = -(uv.y * 2.0 - 1.0) * vh * scale;
			uv.z = -zn;
			src_uv[i] = camera_transform.basis.xform(uv).normalized();
			src_uv[i].z = -src_uv[i].z;
		}
	}

	// _draw_primitive(4, vertices, NULL, NULL, src_uv);

}

/* TEXTURE API */
#define _EXT_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                   0x8C00
#define _EXT_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                   0x8C01
#define _EXT_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                  0x8C02
#define _EXT_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                  0x8C03
#define _EXT_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define _EXT_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define _EXT_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define _EXT_COMPRESSED_RED_RGTC1_EXT 0x8DBB
#define _EXT_COMPRESSED_RED_RGTC1 0x8DBB
#define _EXT_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#define _EXT_COMPRESSED_RG_RGTC2 0x8DBD
#define _EXT_COMPRESSED_SIGNED_RG_RGTC2 0x8DBE
#define _EXT_COMPRESSED_SIGNED_RED_RGTC1_EXT 0x8DBC
#define _EXT_COMPRESSED_RED_GREEN_RGTC2_EXT 0x8DBD
#define _EXT_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT 0x8DBE
#define _EXT_ETC1_RGB8_OES           0x8D64

/* TEXTURE API */

Image RasterizerWIIU::_get_gl_image_and_format(const Image& p_image, Image::Format p_format, uint32_t p_flags,GLenum& r_gl_format,int &r_gl_components,bool &r_has_alpha_cache,bool &r_compressed) {

	r_has_alpha_cache=false;
	r_compressed=false;
	Image image=p_image;

	switch(p_format) {

		case Image::FORMAT_GRAYSCALE: {
			r_gl_components=1;
			r_gl_format=GL_LUMINANCE;

		} break;
		case Image::FORMAT_INTENSITY: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGBA);
			r_gl_components=4;
			r_gl_format=GL_RGBA;
			r_has_alpha_cache=true;
		} break;
		case Image::FORMAT_GRAYSCALE_ALPHA: {

			//image.convert(Image::FORMAT_RGBA);
			r_gl_components=2;
			r_gl_format=GL_LUMINANCE_ALPHA;
			r_has_alpha_cache=true;
		} break;

		case Image::FORMAT_INDEXED: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGB);
			r_gl_components=3;
			r_gl_format=GL_RGB;

		} break;

		case Image::FORMAT_INDEXED_ALPHA: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGBA);
			r_gl_components=4;
			r_gl_format=GL_RGBA;
			r_has_alpha_cache=true;

		} break;
		case Image::FORMAT_RGB: {

			r_gl_components=3;
			r_gl_format=GL_RGB;
		} break;
		case Image::FORMAT_RGBA: {

			r_gl_components=4;
			r_gl_format=GL_RGBA;
			r_has_alpha_cache=true;
		} break;
		case Image::FORMAT_BC1: {

			r_gl_components=1; //doesn't matter much
			r_gl_format=_EXT_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			r_compressed=true;

		} break;
		case Image::FORMAT_BC2: {
			r_gl_components=1; //doesn't matter much
			r_gl_format=_EXT_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			r_has_alpha_cache=true;
			r_compressed=true;

		} break;
		case Image::FORMAT_BC3: {

			r_gl_components=1; //doesn't matter much
			r_gl_format=_EXT_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			r_has_alpha_cache=true;
			r_compressed=true;

		} break;
		case Image::FORMAT_BC4: {

			r_gl_format=_EXT_COMPRESSED_RED_RGTC1;
			r_gl_components=1; //doesn't matter much
			r_compressed=true;

		} break;
		case Image::FORMAT_BC5: {

			r_gl_format=_EXT_COMPRESSED_RG_RGTC2;
			r_gl_components=1; //doesn't matter much
			r_compressed=true;
		} break;
		case Image::FORMAT_PVRTC2: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC2");

			} else {

				r_gl_format=_EXT_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC2");
			}

		} break;
		case Image::FORMAT_PVRTC2_ALPHA: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC2A");

			} else {

				r_gl_format=_EXT_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC2A");
			}

		} break;
		case Image::FORMAT_PVRTC4: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC4");
			} else {

				r_gl_format=_EXT_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC4");
			}

		} break;
		case Image::FORMAT_PVRTC4_ALPHA: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC4A");

			} else {

				r_gl_format=_EXT_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC4A");
			}

		} break;
		case Image::FORMAT_ETC: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
			} else {

				r_gl_format=_EXT_ETC1_RGB8_OES;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
			}

		} break;
		case Image::FORMAT_YUV_422:
		case Image::FORMAT_YUV_444: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGB);
			r_gl_format=GL_RGB;
			r_gl_components=3;

		} break;

		default: {

			ERR_FAIL_V(Image());
		}
	}

	return image;
}


RID RasterizerWIIU::texture_create() {

	Texture *texture = memnew(Texture);
	ERR_FAIL_COND_V(!texture,RID());
	// glGenTextures(1, &texture->tex_id);
	texture->active=false;
	texture->total_data_size=0;

	return texture_owner.make_rid( texture );

}

void RasterizerWIIU::texture_allocate(RID p_texture,int p_width, int p_height,Image::Format p_format,uint32_t p_flags) {

	bool has_alpha_cache;
	int components;
	GLenum format;
	bool compressed;

	int po2_width =  next_power_of_2(p_width);
	int po2_height =  next_power_of_2(p_height);

	Texture *texture = texture_owner.get( p_texture );
	ERR_FAIL_COND(!texture);
	texture->width=p_width;
	texture->height=p_height;
	texture->format=p_format;
	texture->flags=p_flags;
	texture->target = /*(p_flags & VS::TEXTURE_FLAG_CUBEMAP) ? GL_TEXTURE_CUBE_MAP :*/ GL_TEXTURE_2D;

	_get_gl_image_and_format(Image(),texture->format,texture->flags,format,components,has_alpha_cache,compressed);

	bool scale_textures = !compressed; //&& !(p_flags & VS::TEXTURE_FLAG_VIDEO_SURFACE) && (!npo2_textures_available || p_flags & VS::TEXTURE_FLAG_MIPMAPS);


	// if (scale_textures) {
		texture->alloc_width = texture->width;
		texture->alloc_height = texture->height;
		print_line("scale because npo2: "+itos(npo2_textures_available)+" mm: "+itos(p_format&VS::TEXTURE_FLAG_MIPMAPS) );
	/*} else {

		texture->alloc_width = texture->width;
		texture->alloc_height = texture->height;
	};*/

	/*if (!(p_flags & VS::TEXTURE_FLAG_VIDEO_SURFACE)) {
		texture->alloc_height = MAX(1, texture->alloc_height / 2);
		texture->alloc_width = MAX(1, texture->alloc_width / 2);
	}*/

	// texture->gl_components_cache = components;
	texture->gl_format_cache = format;
	// texture->gl_internal_format_cache = internal_format;
	texture->format_has_alpha = has_alpha_cache;
	texture->compressed = compressed;
	// texture->has_alpha = false; //by default it doesn't have alpha unless something with alpha is blitteds
	texture->data_size = 0;
	// texture->mipmaps = 0;

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(texture->target, texture->tex_id);

	if (p_flags & VS::TEXTURE_FLAG_VIDEO_SURFACE) {
		//prealloc if video
		// glTexImage2D(texture->target, 0, format, p_width, p_height, 0, format, GL_UNSIGNED_BYTE, NULL);
	}
	memset(&texture->tex_id, 0, sizeof(GX2Texture));
	// texture->tex_id.surface.use = GX2_SURFACE_USE_TEXTURE;
	texture->tex_id.surface.width    = p_width;
	texture->tex_id.surface.height   = p_height;
    texture->tex_id.surface.depth = 1;
    texture->tex_id.surface.mipLevels = 1;
    texture->tex_id.surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    texture->tex_id.surface.aa = GX2_AA_MODE1X;
    texture->tex_id.surface.use = GX2_SURFACE_USE_TEXTURE;
    texture->tex_id.surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
    texture->tex_id.surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
    texture->tex_id.surface.swizzle = 0;
    texture->tex_id.viewFirstMip = 0;
    texture->tex_id.viewNumMips = 1;
    texture->tex_id.viewFirstSlice = 0;
    texture->tex_id.viewNumSlices = 1;
    texture->tex_id.compMap = 0x0010203;

	GX2CalcSurfaceSizeAndAlignment(&texture->tex_id.surface);
	GX2InitTextureRegs(&texture->tex_id);

	texture->tex_id.surface.image = MEMAllocFromDefaultHeapEx(texture->tex_id.surface.imageSize, texture->tex_id.surface.alignment);
	// texture->tex_id.surface.image = memalign(texture->tex_id.surface.alignment, texture->tex_id.surface.imageSize);




	texture->active = true;
}

void RasterizerWIIU::texture_set_data(RID p_texture,const Image& p_image,VS::CubeMapSide p_cube_side) {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND(!texture);
	ERR_FAIL_COND(!texture->active);
	ERR_FAIL_COND(texture->format != p_image.get_format() );

	int components;
	GLenum format;
	bool alpha;
	bool compressed;

	if (keep_copies && !(texture->flags&VS::TEXTURE_FLAG_VIDEO_SURFACE) && !(use_reload_hooks && texture->reloader)) {
		texture->image[p_cube_side]=p_image;
	}


	Image img = _get_gl_image_and_format(p_image, p_image.get_format(),texture->flags,format,components,alpha,compressed);
	if (texture->alloc_width != img.get_width() || texture->alloc_height != img.get_height()) {

		img.resize(texture->alloc_width, texture->alloc_height, Image::INTERPOLATE_BILINEAR);
		img.convert(Image::FORMAT_RGBA);
	};


	GLenum blit_target = /*(texture->target == GL_TEXTURE_CUBE_MAP)?_cube_side_enum[p_cube_side]:*/GL_TEXTURE_2D;

	texture->data_size=img.get_data().size();
	DVector<uint8_t>::Read read = img.get_data().read();

	//glActiveTexture(GL_TEXTURE0);
	// glBindTexture(texture->target, texture->tex_id);

	int mipmaps=(texture->flags&VS::TEXTURE_FLAG_MIPMAPS && img.get_mipmaps()>0) ? img.get_mipmaps() +1 : 1;

	int w=img.get_width();
	int h=img.get_height();

	int tsize=0;
	for(int i=0;i<mipmaps;i++) {

		int size,ofs;
		img.get_mipmap_offset_and_size(i,ofs,size);

		if (texture->compressed) {
			// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			// glTexImage2D( blit_target, i, format,w,h,0,format,GL_UNSIGNED_BYTE,&read[ofs] );

		} else {

			// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//			glTexImage2D(blit_target, i, format==GL_RGB?GL_RGB8:format, w, h, 0, format, GL_UNSIGNED_BYTE,&read[ofs]);
			// glTexImage2D(blit_target, i, format, w, h, 0, format, GL_UNSIGNED_BYTE,&read[ofs]);
			//glTexSubImage2D( blit_target, i, 0,0,w,h,format,GL_UNSIGNED_BYTE,&read[ofs] );
		}

/*
		 const unsigned char* src = &read[ofs];
         uint32_t* dst = (uint32_t*)texture->tex_id.surface.image;

         for (i = 0; i < h; i++)
         {
            int j;
            for(j = 0; j < w; j++)
               dst[j] = src[j];
            dst += texture->tex_id.surface.pitch;
            src += (w );
         }*/
	memcpy(texture->tex_id.surface.image, &read[ofs], texture->tex_id.surface.imageSize);
	GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_TEXTURE), texture->tex_id.surface.image, texture->tex_id.surface.imageSize);


		tsize+=size;

		w = MAX(1,w>>1);
		h = MAX(1,h>>1);

	}

	_rinfo.texture_mem-=texture->total_data_size;
	texture->total_data_size=tsize;
	_rinfo.texture_mem+=texture->total_data_size;

	OSReport("texture: %i x %i - size: %i, godot_size %i - total: %i\n",texture->width,texture->height, texture->tex_id.surface.imageSize, tsize, _rinfo.texture_mem);


	if (mipmaps==1 && texture->flags&VS::TEXTURE_FLAG_MIPMAPS) {
		// glTexParameterf( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

	} else {
		// glTexParameterf( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );

	}

	if (mipmaps>1) {

	// const u32* imagedata = reinterpret_cast<const u32*>(d.read().ptr());
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmaps-1 ); - assumed to have all, always
	}

	//texture_set_flags(p_texture,texture->flags);
	// texture->tex_id.surface.image = (void*)&read[0];

	// GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, texture->tex_id.surface.image, texture->tex_id.surface.imageSize);
	// memset(texture->tex_id.surface.image, 0, h *  texture->tex_id.surface.pitch);

	// uint8_t* buf = (uint8_t*)texture->tex_id.surface.image;
 //    for (uint32_t y = 0; y < h; ++y) {
 //        memcpy(buf + (y * texture->tex_id.surface.pitch ), &read + (y * w), w);
 //    }

	// GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU_TEXTURE), texture->tex_id.surface.image, texture->tex_id.surface.imageSize);
	// GFDGetTexture(&texture->tex_id, &read, nullptr, 0, 0);

}

Image RasterizerWIIU::texture_get_data(RID p_texture,VS::CubeMapSide p_cube_side) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,Image());
	ERR_FAIL_COND_V(!texture->active,Image());

	return texture->image[p_cube_side];
#if 0

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,Image());
	ERR_FAIL_COND_V(!texture->active,Image());
	ERR_FAIL_COND_V(texture->data_size==0,Image());

	DVector<uint8_t> data;
	GLenum format,type=GL_UNSIGNED_BYTE;
	Image::Format fmt;
	int pixelsize=0;
	int pixelshift=0;
	int minw=1,minh=1;
	bool compressed=false;

	fmt=texture->format;

	switch(texture->format) {

		case Image::FORMAT_GRAYSCALE: {

			format=GL_LUMINANCE;
			type=GL_UNSIGNED_BYTE;
			data.resize(texture->alloc_width*texture->alloc_height);
			pixelsize=1;

		} break;
		case Image::FORMAT_INTENSITY: {
			return Image();
		} break;
		case Image::FORMAT_GRAYSCALE_ALPHA: {

			format=GL_LUMINANCE_ALPHA;
			type=GL_UNSIGNED_BYTE;
			pixelsize=2;

		} break;
		case Image::FORMAT_RGB: {
			format=GL_RGB;
			type=GL_UNSIGNED_BYTE;
			pixelsize=3;
		} break;
		case Image::FORMAT_RGBA: {

			format=GL_RGBA;
			type=GL_UNSIGNED_BYTE;
			pixelsize=4;
		} break;
		case Image::FORMAT_INDEXED: {

			format=GL_RGB;
			type=GL_UNSIGNED_BYTE;
			fmt=Image::FORMAT_RGB;
			pixelsize=3;
		} break;
		case Image::FORMAT_INDEXED_ALPHA: {

			format=GL_RGBA;
			type=GL_UNSIGNED_BYTE;
			fmt=Image::FORMAT_RGBA;
			pixelsize=4;

		} break;
		case Image::FORMAT_BC1: {

			pixelsize=1; //doesn't matter much
			format=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			compressed=true;
			pixelshift=1;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC2: {
			pixelsize=1; //doesn't matter much
			format=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			compressed=true;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC3: {

			pixelsize=1; //doesn't matter much
			format=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			compressed=true;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC4: {

			format=GL_COMPRESSED_RED_RGTC1;
			pixelsize=1; //doesn't matter much
			compressed=true;
			pixelshift=1;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC5: {

			format=GL_COMPRESSED_RG_RGTC2;
			pixelsize=1; //doesn't matter much
			compressed=true;
			minw=minh=4;

		} break;

		default:{}
	}

	data.resize(texture->data_size);
	DVector<uint8_t>::Write wb = data.write();

	//glActiveTexture(GL_TEXTURE0);
	int ofs=0;
	glBindTexture(texture->target,texture->tex_id);

	int w=texture->alloc_width;
	int h=texture->alloc_height;
	for(int i=0;i<texture->mipmaps+1;i++) {

		if (compressed) {

			glPixelStorei(GL_PACK_ALIGNMENT, 4);
			glGetCompressedTexImage(texture->target,i,&wb[ofs]);

		} else {
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glGetTexImage(texture->target,i,format,type,&wb[ofs]);
		}

		int size = (w*h*pixelsize)>>pixelshift;
		ofs+=size;

		w=MAX(minw,w>>1);
		h=MAX(minh,h>>1);

	}


	wb=DVector<uint8_t>::Write();

	Image img(texture->alloc_width,texture->alloc_height,texture->mipmaps,fmt,data);

	if (texture->format<Image::FORMAT_INDEXED && (texture->alloc_width!=texture->width || texture->alloc_height!=texture->height))
		img.resize(texture->width,texture->height);

	return img;
#endif
}

void RasterizerWIIU::texture_set_flags(RID p_texture,uint32_t p_flags) {

	Texture *texture = texture_owner.get( p_texture );
	ERR_FAIL_COND(!texture);

	//glActiveTexture(GL_TEXTURE0);
	// glBindTexture(texture->target, texture->tex_id);
	uint32_t cube = texture->flags & VS::TEXTURE_FLAG_CUBEMAP;
	texture->flags=p_flags|cube; // can't remove a cube from being a cube

	bool force_clamp_to_edge = !(p_flags&VS::TEXTURE_FLAG_MIPMAPS) && (next_power_of_2(texture->alloc_height)!=texture->alloc_height || next_power_of_2(texture->alloc_width)!=texture->alloc_width);

	if (!force_clamp_to_edge && texture->flags&VS::TEXTURE_FLAG_REPEAT) {

		// glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		// glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	} else {
		//glTexParameterf( texture->target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
		// glTexParameterf( texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		// glTexParameterf( texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	}


	if (texture->flags&VS::TEXTURE_FLAG_FILTER) {

		// glTexParameterf(texture->target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
		// if (texture->flags&VS::TEXTURE_FLAG_MIPMAPS)
			// glTexParameterf(texture->target,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		// else
			// glTexParameterf(texture->target,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering

	} else {

		// glTexParameterf(texture->target,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// nearest
	}
}
uint32_t RasterizerWIIU::texture_get_flags(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return texture->flags;

}
Image::Format RasterizerWIIU::texture_get_format(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,Image::FORMAT_GRAYSCALE);

	return texture->format;
}
uint32_t RasterizerWIIU::texture_get_width(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return texture->width;
}
uint32_t RasterizerWIIU::texture_get_height(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return texture->height;
}

bool RasterizerWIIU::texture_has_alpha(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return false;

}

void RasterizerWIIU::texture_set_size_override(RID p_texture,int p_width, int p_height) {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND(!texture);

	ERR_FAIL_COND(p_width<=0 || p_width>4096);
	ERR_FAIL_COND(p_height<=0 || p_height>4096);
	//real texture size is in alloc width and height
	texture->width=p_width;
	texture->height=p_height;

}

void RasterizerWIIU::texture_set_reload_hook(RID p_texture,ObjectID p_owner,const StringName& p_function) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND(!texture);

	texture->reloader=p_owner;
	texture->reloader_func=p_function;
	if (use_reload_hooks && p_owner && keep_copies) {

		for(int i=0;i<6;i++)
			texture->image[i]=Image();
	}


}

/* SHADER API */

/* SHADER API */

RID RasterizerWIIU::shader_create(VS::ShaderMode p_mode) {

	Shader *shader = memnew( Shader );
	shader->mode=p_mode;
	shader->valid=false;
	shader->has_alpha=false;
	shader->fragment_line=0;
	shader->vertex_line=0;
	shader->light_line=0;
	RID rid = shader_owner.make_rid(shader);
	shader_set_mode(rid,p_mode);
//	_shader_make_dirty(shader);

	return rid;

}



void RasterizerWIIU::shader_set_mode(RID p_shader,VS::ShaderMode p_mode) {

	ERR_FAIL_INDEX(p_mode,3);
	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);
//	if (shader->custom_code_id && p_mode==shader->mode)
//		return;

	shader->mode=p_mode;

}
VS::ShaderMode RasterizerWIIU::shader_get_mode(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,VS::SHADER_MATERIAL);
	return shader->mode;
}



void RasterizerWIIU::shader_set_code(RID p_shader, const String& p_vertex, const String& p_fragment,const String& p_light,int p_vertex_ofs,int p_fragment_ofs,int p_light_ofs) {


	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);

#ifdef DEBUG_ENABLED
	if (shader->vertex_code==p_vertex && shader->fragment_code==p_fragment && shader->light_code==p_light)
		return;
#endif
	shader->fragment_code=p_fragment;
	shader->vertex_code=p_vertex;
	shader->light_code=p_light;
	shader->fragment_line=p_fragment_ofs;
	shader->vertex_line=p_vertex_ofs;
	shader->light_line=p_light_ofs;

}

String RasterizerWIIU::shader_get_vertex_code(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,String());
	return shader->vertex_code;

}

String RasterizerWIIU::shader_get_fragment_code(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,String());
	return shader->fragment_code;

}

String RasterizerWIIU::shader_get_light_code(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,String());
	return shader->light_code;

}

void RasterizerWIIU::shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);
#if 0

	if (shader->dirty_list.in_list())
		_update_shader(shader); // ok should be not anymore dirty


	Map<int,StringName> order;


	for(Map<StringName,ShaderLanguage::Uniform>::Element *E=shader->uniforms.front();E;E=E->next()) {


		order[E->get().order]=E->key();
	}


	for(Map<int,StringName>::Element *E=order.front();E;E=E->next()) {

		PropertyInfo pi;
		ShaderLanguage::Uniform &u=shader->uniforms[E->get()];
		pi.name=E->get();
		switch(u.type) {

			case ShaderLanguage::TYPE_VOID:
			case ShaderLanguage::TYPE_BOOL:
			case ShaderLanguage::TYPE_FLOAT:
			case ShaderLanguage::TYPE_VEC2:
			case ShaderLanguage::TYPE_VEC3:
			case ShaderLanguage::TYPE_MAT3:
			case ShaderLanguage::TYPE_MAT4:
			case ShaderLanguage::TYPE_VEC4:
				pi.type=u.default_value.get_type();
				break;
			case ShaderLanguage::TYPE_TEXTURE:
				pi.type=Variant::_RID;
				pi.hint=PROPERTY_HINT_RESOURCE_TYPE;
				pi.hint_string="Texture";
				break;
			case ShaderLanguage::TYPE_CUBEMAP:
				pi.type=Variant::_RID;
				pi.hint=PROPERTY_HINT_RESOURCE_TYPE;
				pi.hint_string="Texture";
				break;
		};

		p_param_list->push_back(pi);

	}
#endif

}

/* COMMON MATERIAL API */


RID RasterizerWIIU::material_create() {

	return material_owner.make_rid( memnew( Material ) );
}

void RasterizerWIIU::material_set_shader(RID p_material, RID p_shader) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->shader=p_shader;

}

RID RasterizerWIIU::material_get_shader(RID p_material) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,RID());
	return material->shader;
}

#if 0

void RasterizerWIIU::_material_check_alpha(Material *p_material) {

	p_material->has_alpha=false;
	Color diffuse=p_material->parameters[VS::FIXED_MATERIAL_PARAM_DIFFUSE];
	if (diffuse.a<0.98) {

		p_material->has_alpha=true;
		return;
	}

	if (p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE].is_valid()) {

		Texture *tex = texture_owner.get(p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE]);
		if (!tex)
			return;
		if (tex->has_alpha) {

			p_material->has_alpha=true;
			return;
		}
	}
}

#endif
void RasterizerWIIU::material_set_param(RID p_material, const StringName& p_param, const Variant& p_value) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);

	if (p_value.get_type()==Variant::NIL)
		material->shader_params.erase(p_param);
	else
		material->shader_params[p_param]=p_value;
}
Variant RasterizerWIIU::material_get_param(RID p_material, const StringName& p_param) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,Variant());

	if (material->shader_params.has(p_param))
		return material->shader_params[p_param];
	else
		return Variant();
}


void RasterizerWIIU::material_set_flag(RID p_material, VS::MaterialFlag p_flag,bool p_enabled) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	ERR_FAIL_INDEX(p_flag,VS::MATERIAL_FLAG_MAX);
	material->flags[p_flag]=p_enabled;

}
bool RasterizerWIIU::material_get_flag(RID p_material,VS::MaterialFlag p_flag) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,false);
	ERR_FAIL_INDEX_V(p_flag,VS::MATERIAL_FLAG_MAX,false);
	return material->flags[p_flag];


}

void RasterizerWIIU::material_set_depth_draw_mode(RID p_material, VS::MaterialDepthDrawMode p_mode) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->depth_draw_mode=p_mode;
}

VS::MaterialDepthDrawMode RasterizerWIIU::material_get_depth_draw_mode(RID p_material) const{


	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,VS::MATERIAL_DEPTH_DRAW_ALWAYS);
	return material->depth_draw_mode;
}


void RasterizerWIIU::material_set_blend_mode(RID p_material,VS::MaterialBlendMode p_mode) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->blend_mode=p_mode;

}
VS::MaterialBlendMode RasterizerWIIU::material_get_blend_mode(RID p_material) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,VS::MATERIAL_BLEND_MODE_ADD);
	return material->blend_mode;
}

void RasterizerWIIU::material_set_line_width(RID p_material,float p_line_width) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->line_width=p_line_width;

}
float RasterizerWIIU::material_get_line_width(RID p_material) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,0);

	return material->line_width;
}

/* FIXED MATERIAL */


RID RasterizerWIIU::fixed_material_create() {

	return material_create();
}

void RasterizerWIIU::fixed_material_set_flag(RID p_material, VS::FixedMaterialFlags p_flag, bool p_enabled) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_flag, VS::MATERIAL_FLAG_MAX);
	m->fixed_flags[p_flag]=p_enabled;
}

bool RasterizerWIIU::fixed_material_get_flag(RID p_material, VS::FixedMaterialFlags p_flag) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m,false);
	ERR_FAIL_INDEX_V(p_flag,VS::FIXED_MATERIAL_FLAG_MAX, false);
	return m->fixed_flags[p_flag];
}

void RasterizerWIIU::fixed_material_set_parameter(RID p_material, VS::FixedMaterialParam p_parameter, const Variant& p_value) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX);

	m->parameters[p_parameter] = p_value;

}

Variant RasterizerWIIU::fixed_material_get_parameter(RID p_material,VS::FixedMaterialParam p_parameter) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, Variant());
	ERR_FAIL_INDEX_V(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX, Variant());

	return m->parameters[p_parameter];
}

void RasterizerWIIU::fixed_material_set_texture(RID p_material,VS::FixedMaterialParam p_parameter, RID p_texture) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX);

	m->textures[p_parameter] = p_texture;

}
RID RasterizerWIIU::fixed_material_get_texture(RID p_material,VS::FixedMaterialParam p_parameter) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, RID());
	ERR_FAIL_INDEX_V(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX, Variant());

	return m->textures[p_parameter];
}


void RasterizerWIIU::fixed_material_set_texcoord_mode(RID p_material,VS::FixedMaterialParam p_parameter, VS::FixedMaterialTexCoordMode p_mode) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX);
	ERR_FAIL_INDEX(p_mode,4);

	m->texcoord_mode[p_parameter] = p_mode;
}

VS::FixedMaterialTexCoordMode RasterizerWIIU::fixed_material_get_texcoord_mode(RID p_material,VS::FixedMaterialParam p_parameter) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, VS::FIXED_MATERIAL_TEXCOORD_UV);
	ERR_FAIL_INDEX_V(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX, VS::FIXED_MATERIAL_TEXCOORD_UV);

	return m->texcoord_mode[p_parameter]; // for now
}

void RasterizerWIIU::fixed_material_set_uv_transform(RID p_material,const Transform& p_transform) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);

	m->uv_transform = p_transform;
}

Transform RasterizerWIIU::fixed_material_get_uv_transform(RID p_material) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, Transform());

	return m->uv_transform;
}

void RasterizerWIIU::fixed_material_set_point_size(RID p_material,float p_size) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	m->point_size=p_size;

}
float RasterizerWIIU::fixed_material_get_point_size(RID p_material) const {

	const Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, 0);
	return m->point_size;
}


/* MESH API */


RID RasterizerWIIU::mesh_create() {


	return mesh_owner.make_rid( memnew( Mesh ) );
}



void RasterizerWIIU::mesh_add_surface(RID p_mesh,VS::PrimitiveType p_primitive,const Array& p_arrays,const Array& p_blend_shapes,bool p_alpha_sort) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);

	ERR_FAIL_INDEX( p_primitive, VS::PRIMITIVE_MAX );
	ERR_FAIL_COND(p_arrays.size()!=VS::ARRAY_MAX);

	uint32_t format=0;

	// validation
	int index_array_len=0;
	int array_len=0;

	for(int i=0;i<p_arrays.size();i++) {

		if (p_arrays[i].get_type()==Variant::NIL)
			continue;

		format|=(1<<i);

		if (i==VS::ARRAY_VERTEX) {

			array_len=Vector3Array(p_arrays[i]).size();
			ERR_FAIL_COND(array_len==0);
		} else if (i==VS::ARRAY_INDEX) {

			index_array_len=IntArray(p_arrays[i]).size();
		}
	}

	ERR_FAIL_COND((format&VS::ARRAY_FORMAT_VERTEX)==0); // mandatory


	Surface *surface = memnew( Surface );
	ERR_FAIL_COND( !surface );

	bool use_VBO=true; //glGenBuffersARB!=NULL; // TODO detect if it's in there
	if (format&VS::ARRAY_FORMAT_WEIGHTS || mesh->morph_target_count>0) {

		use_VBO=false;
	}

	surface->packed=pack_arrays && use_VBO;

	int total_elem_size=0;

	for (int i=0;i<VS::ARRAY_MAX;i++) {


		Surface::ArrayData&ad=surface->array[i];
		ad.size=0;
		ad.ofs=0;
		int elem_size=0;
		int elem_count=0;
		bool valid_local=true;
		GLenum datatype;
		bool normalize=false;
		bool bind=false;

		if (!(format&(1<<i))) // no array
			continue;


		switch(i) {

			case VS::ARRAY_VERTEX: {

				if (surface->packed) {
					elem_size=3*sizeof(int16_t); // vertex
					datatype=GL_SHORT;
					normalize=true;

				} else {
					elem_size=3*sizeof(GLfloat); // vertex
					datatype=GL_FLOAT;
				}
				bind=true;
				elem_count=3;

			} break;
			case VS::ARRAY_NORMAL: {

				if (surface->packed) {
					elem_size=3*sizeof(int8_t); // vertex
					datatype=GL_BYTE;
					normalize=true;
				} else {
					elem_size=3*sizeof(GLfloat); // vertex
					datatype=GL_FLOAT;
				}
				bind=true;
				elem_count=3;
			} break;
			case VS::ARRAY_TANGENT: {
				if (surface->packed) {
					elem_size=4*sizeof(int8_t); // vertex
					datatype=GL_BYTE;
					normalize=true;
				} else {
					elem_size=4*sizeof(GLfloat); // vertex
					datatype=GL_FLOAT;
				}
				bind=true;
				elem_count=4;

			} break;
			case VS::ARRAY_COLOR: {

				elem_size=4*sizeof(uint8_t); /* RGBA */
				datatype=GL_UNSIGNED_BYTE;
				elem_count=4;
				bind=true;
				normalize=true;
			} break;
			case VS::ARRAY_TEX_UV:
			case VS::ARRAY_TEX_UV2: {
				if (surface->packed) {
					elem_size=2*sizeof(int16_t); // vertex
					datatype=GL_SHORT;
					normalize=true;
				} else {
					elem_size=2*sizeof(GLfloat); // vertex
					datatype=GL_FLOAT;
				}
				bind=true;
				elem_count=2;

			} break;
			case VS::ARRAY_WEIGHTS: {

				elem_size=VS::ARRAY_WEIGHTS_SIZE*sizeof(GLfloat);
				elem_count=VS::ARRAY_WEIGHTS_SIZE;
				valid_local=false;
				datatype=GL_FLOAT;

			} break;
			case VS::ARRAY_BONES: {

				elem_size=VS::ARRAY_WEIGHTS_SIZE*sizeof(GLuint);
				elem_count=VS::ARRAY_WEIGHTS_SIZE;
				valid_local=false;
				datatype=GL_FLOAT;


			} break;
			case VS::ARRAY_INDEX: {

				if (index_array_len<=0) {
					ERR_PRINT("index_array_len==NO_INDEX_ARRAY");
					break;
				}
				/* determine wether using 16 or 32 bits indices */
				elem_size=2;
				datatype=GL_UNSIGNED_SHORT;

/*
				if (use_VBO) {
					glGenBuffers(1,&surface->index_id);
					ERR_FAIL_COND(surface->index_id==0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,surface->index_id);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER,index_array_len*elem_size,NULL,GL_STATIC_DRAW);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0); //unbind
				} else {
					surface->index_array_local = (uint8_t*)memalloc(index_array_len*elem_size);
				};
*/
				surface->index_array_len=index_array_len; // only way it can exist
				ad.ofs=0;
				ad.size=elem_size;


				continue;
			} break;
			default: {
				ERR_FAIL( );
			}
		}

		ad.ofs=total_elem_size;
		ad.size=elem_size;
		ad.datatype=datatype;
		ad.normalize=normalize;
		ad.bind=bind;
		ad.count=elem_count;
		total_elem_size+=elem_size;
		if (valid_local) {
			surface->local_stride+=elem_size;
			surface->morph_format|=(1<<i);
		}


	}

	surface->stride=total_elem_size;
	surface->array_len=array_len;
	surface->format=format;
	surface->primitive=p_primitive;
	surface->configured_format=0;
	if (keep_copies) {
		surface->data=p_arrays;
		surface->morph_data=p_blend_shapes;
	}

	uint8_t *array_ptr=NULL;
	uint8_t *index_array_ptr=NULL;
	DVector<uint8_t> array_pre_vbo;
	DVector<uint8_t>::Write vaw;
	DVector<uint8_t> index_array_pre_vbo;
	DVector<uint8_t>::Write iaw;

	/* create pointers */
	if (use_VBO) {

		array_pre_vbo.resize(surface->array_len*surface->stride);
		vaw = array_pre_vbo.write();
		array_ptr=vaw.ptr();

		if (surface->index_array_len) {

			index_array_pre_vbo.resize(surface->index_array_len*surface->array[VS::ARRAY_INDEX].size);
			iaw = index_array_pre_vbo.write();
			index_array_ptr=iaw.ptr();
		}
	} else {

		surface->array_local = (uint8_t*)memalloc(surface->array_len*surface->stride);
		array_ptr=(uint8_t*)surface->array_local;
		if (surface->index_array_len) {
			surface->index_array_local = (uint8_t*)memalloc(index_array_len*surface->array[VS::ARRAY_INDEX].size);
			index_array_ptr=(uint8_t*)surface->index_array_local;
		}
	}



	_surface_set_arrays(surface,array_ptr,index_array_ptr,p_arrays,true);


	/* create buffers!! */
	if (use_VBO) {
		// glGenBuffers(1,&surface->vertex_id);
		ERR_FAIL_COND(surface->vertex_id==0);
		// glBindBuffer(GL_ARRAY_BUFFER,surface->vertex_id);
		// glBufferData(GL_ARRAY_BUFFER,surface->array_len*surface->stride,array_ptr,GL_STATIC_DRAW);
		// glBindBuffer(GL_ARRAY_BUFFER,0); //unbind
		if (surface->index_array_len) {

			// glGenBuffers(1,&surface->index_id);
			ERR_FAIL_COND(surface->index_id==0);
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,surface->index_id);
			// glBufferData(GL_ELEMENT_ARRAY_BUFFER,index_array_len*surface->array[VS::ARRAY_INDEX].size,index_array_ptr,GL_STATIC_DRAW);
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0); //unbind

		}
	}

	mesh->surfaces.push_back(surface);

}

Error RasterizerWIIU::_surface_set_arrays(Surface *p_surface, uint8_t *p_mem,uint8_t *p_index_mem,const Array& p_arrays,bool p_main) {

	uint32_t stride = p_main ? p_surface->stride : p_surface->local_stride;

	for(int ai=0;ai<VS::ARRAY_MAX;ai++) {
		if (ai>=p_arrays.size())
			break;
		if (p_arrays[ai].get_type()==Variant::NIL)
			continue;
		Surface::ArrayData &a=p_surface->array[ai];

		switch(ai) {


			case VS::ARRAY_VERTEX: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::VECTOR3_ARRAY, ERR_INVALID_PARAMETER );

				DVector<Vector3> array = p_arrays[ai];
				ERR_FAIL_COND_V( array.size() != p_surface->array_len, ERR_INVALID_PARAMETER );


				DVector<Vector3>::Read read = array.read();
				const Vector3* src=read.ptr();

				// setting vertices means regenerating the AABB
				AABB aabb;

				float scale=1;
				float max=0;


				for (int i=0;i<p_surface->array_len;i++) {


					GLfloat vector[3]={ src[i].x, src[i].y, src[i].z };

					copymem(&p_mem[a.ofs+i*stride], vector, a.size);

					if (i==0) {

						aabb=AABB(src[i],Vector3());
					} else {

						aabb.expand_to( src[i] );
					}
				}

				if (p_main) {
					p_surface->aabb=aabb;
					p_surface->vertex_scale=scale;
				}


			} break;
			case VS::ARRAY_NORMAL: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::VECTOR3_ARRAY, ERR_INVALID_PARAMETER );

				DVector<Vector3> array = p_arrays[ai];
				ERR_FAIL_COND_V( array.size() != p_surface->array_len, ERR_INVALID_PARAMETER );


				DVector<Vector3>::Read read = array.read();
				const Vector3* src=read.ptr();

				// setting vertices means regenerating the AABB


				if (p_surface->array[VS::ARRAY_NORMAL].datatype == GL_BYTE) {

					for (int i = 0; i < p_surface->array_len; i++) {

						GLbyte vector[4] = {
							(GLbyte)CLAMP(src[i].x * 127, -128, 127),
							(GLbyte)CLAMP(src[i].y * 127, -128, 127),
							(GLbyte)CLAMP(src[i].z * 127, -128, 127),
							0,
						};

						copymem(&p_mem[a.ofs + i * stride], vector, a.size);
					}

				} else {
					for (int i = 0; i < p_surface->array_len; i++) {


						GLfloat vector[3]={ src[i].x, src[i].y, src[i].z };
						copymem(&p_mem[a.ofs+i*stride], vector, a.size);

					}
				}


			} break;
			case VS::ARRAY_TANGENT: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::REAL_ARRAY, ERR_INVALID_PARAMETER );

				DVector<real_t> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len*4, ERR_INVALID_PARAMETER );


				DVector<real_t>::Read read = array.read();
				const real_t* src = read.ptr();

				if (p_surface->array[VS::ARRAY_TANGENT].datatype == GL_BYTE) {

					for (int i = 0; i < p_surface->array_len; i++) {

						GLbyte xyzw[4] = {
							(GLbyte)CLAMP(src[i * 4 + 0] * 127, -128, 127),
							(GLbyte)CLAMP(src[i * 4 + 1] * 127, -128, 127),
							(GLbyte)CLAMP(src[i * 4 + 2] * 127, -128, 127),
							(GLbyte)CLAMP(src[i * 4 + 3] * 127, -128, 127)
						};

						copymem(&p_mem[a.ofs + i * stride], xyzw, a.size);
					}

				} else {
					for (int i = 0; i < p_surface->array_len; i++) {
					GLfloat xyzw[4]={
						src[i*4+0],
						src[i*4+1],
						src[i*4+2],
						src[i*4+3]
					};

					copymem(&p_mem[a.ofs+i*stride], xyzw, a.size);

				}
			}

			} break;
			case VS::ARRAY_COLOR: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::COLOR_ARRAY, ERR_INVALID_PARAMETER );


				DVector<Color> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len, ERR_INVALID_PARAMETER );


				DVector<Color>::Read read = array.read();
				const Color* src = read.ptr();
				bool alpha=false;

				for (int i=0;i<p_surface->array_len;i++) {

					if (src[i].a<0.98) // tolerate alpha a bit, for crappy exporters
						alpha=true;

					uint8_t colors[4];

					for(int j=0;j<4;j++) {

						colors[j]=CLAMP( int((src[i][j])*255.0), 0,255 );
					}

						copymem(&p_mem[a.ofs+i*stride], colors, a.size);

				}

				if (p_main)
					p_surface->has_alpha=alpha;

			} break;
			case VS::ARRAY_TEX_UV:
			case VS::ARRAY_TEX_UV2: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::VECTOR3_ARRAY && p_arrays[ai].get_type() != Variant::VECTOR2_ARRAY, ERR_INVALID_PARAMETER );

				DVector<Vector2> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len , ERR_INVALID_PARAMETER);

				DVector<Vector2>::Read read = array.read();

				const Vector2 * src=read.ptr();
				float scale=1.0;

// 				if (p_surface->array[ai].datatype == _GL_HALF_FLOAT_OES) {
//
// 					for (int i = 0; i < p_surface->array_len; i++) {
//
// 						uint16_t uv[2] = { make_half_float(src[i].x), make_half_float(src[i].y) };
// 						copymem(&p_mem[a.ofs + i * stride], uv, a.size);
// 					}
//
// 				} else {
				for (int i=0;i<p_surface->array_len;i++) {

					GLfloat uv[2]={ src[i].x , src[i].y };

					copymem(&p_mem[a.ofs+i*stride], uv, a.size);
// 				}
				}

				if (p_main) {

					if  (ai==VS::ARRAY_TEX_UV) {

						p_surface->uv_scale=scale;
					}
					if  (ai==VS::ARRAY_TEX_UV2) {

						p_surface->uv2_scale=scale;
					}
				}

			} break;
			case VS::ARRAY_BONES:
			case VS::ARRAY_WEIGHTS: {


				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::REAL_ARRAY, ERR_INVALID_PARAMETER );

				DVector<real_t> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len*VS::ARRAY_WEIGHTS_SIZE, ERR_INVALID_PARAMETER );


				DVector<real_t>::Read read = array.read();

				const real_t * src = read.ptr();

				p_surface->max_bone=0;

				for (int i=0;i<p_surface->array_len;i++) {

					GLfloat data[VS::ARRAY_WEIGHTS_SIZE];
					for (int j=0;j<VS::ARRAY_WEIGHTS_SIZE;j++) {
						data[j]=src[i*VS::ARRAY_WEIGHTS_SIZE+j];
						if (ai==VS::ARRAY_BONES) {

							p_surface->max_bone=MAX(data[j],p_surface->max_bone);
						}
					}

					copymem(&p_mem[a.ofs+i*stride], data, a.size);


				}

			} break;
			case VS::ARRAY_INDEX: {

				ERR_FAIL_COND_V( p_surface->index_array_len<=0, ERR_INVALID_DATA );
				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::INT_ARRAY, ERR_INVALID_PARAMETER );

				DVector<int> indices = p_arrays[ai];
				ERR_FAIL_COND_V( indices.size() == 0, ERR_INVALID_PARAMETER );
				ERR_FAIL_COND_V( indices.size() != p_surface->index_array_len, ERR_INVALID_PARAMETER );

				/* determine wether using 16 or 32 bits indices */

				DVector<int>::Read read = indices.read();
				const int *src=read.ptr();

				for (int i=0;i<p_surface->index_array_len;i++) {


					if (a.size==2) {
						uint16_t v=src[i];

						copymem(&p_index_mem[i*a.size], &v, a.size);
					} else {
						uint32_t v=src[i];

						copymem(&p_index_mem[i*a.size], &v, a.size);
					}
				}


			} break;


			default: { ERR_FAIL_V(ERR_INVALID_PARAMETER);}
		}

		p_surface->configured_format|=(1<<ai);
	}

	return OK;
}



void RasterizerWIIU::mesh_add_custom_surface(RID p_mesh,const Variant& p_dat) {

	ERR_EXPLAIN("OpenGL Rasterizer does not support custom surfaces. Running on wrong platform?");
	ERR_FAIL_V();
}

Array RasterizerWIIU::mesh_get_surface_arrays(RID p_mesh,int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,Array());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), Array() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, Array() );

	return surface->data;


}
Array RasterizerWIIU::mesh_get_surface_morph_arrays(RID p_mesh,int p_surface) const{

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,Array());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), Array() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, Array() );

	return surface->morph_data;

}


void RasterizerWIIU::mesh_set_morph_target_count(RID p_mesh,int p_amount) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_COND( mesh->surfaces.size()!=0 );

	mesh->morph_target_count=p_amount;

}

int RasterizerWIIU::mesh_get_morph_target_count(RID p_mesh) const{

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);

	return mesh->morph_target_count;

}

void RasterizerWIIU::mesh_set_morph_target_mode(RID p_mesh,VS::MorphTargetMode p_mode) {

	ERR_FAIL_INDEX(p_mode,2);
	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);

	mesh->morph_target_mode=p_mode;

}

VS::MorphTargetMode RasterizerWIIU::mesh_get_morph_target_mode(RID p_mesh) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,VS::MORPH_MODE_NORMALIZED);

	return mesh->morph_target_mode;

}



void RasterizerWIIU::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material,bool p_owned) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_INDEX(p_surface, mesh->surfaces.size() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND( !surface);

	if (surface->material_owned && surface->material.is_valid())
		free(surface->material);

	surface->material_owned=p_owned;

	surface->material=p_material;
}

RID RasterizerWIIU::mesh_surface_get_material(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,RID());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), RID() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, RID() );

	return surface->material;
}

int RasterizerWIIU::mesh_surface_get_array_len(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), -1 );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, -1 );

	return surface->array_len;
}
int RasterizerWIIU::mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), -1 );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, -1 );

	return surface->index_array_len;
}
uint32_t RasterizerWIIU::mesh_surface_get_format(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,0);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), 0 );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, 0 );

	return surface->format;
}
VS::PrimitiveType RasterizerWIIU::mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,VS::PRIMITIVE_POINTS);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), VS::PRIMITIVE_POINTS );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, VS::PRIMITIVE_POINTS );

	return surface->primitive;
}

void RasterizerWIIU::mesh_remove_surface(RID p_mesh,int p_index) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_INDEX(p_index, mesh->surfaces.size() );
	Surface *surface = mesh->surfaces[p_index];
	ERR_FAIL_COND( !surface);

	if (mesh->morph_target_count) {
		for(int i=0;i<mesh->morph_target_count;i++)
			memfree(surface->morph_targets_local[i].array);
		memfree( surface->morph_targets_local );
	}

	memdelete( mesh->surfaces[p_index] );
	mesh->surfaces.remove(p_index);

}
int RasterizerWIIU::mesh_get_surface_count(RID p_mesh) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);

	return mesh->surfaces.size();
}

AABB RasterizerWIIU::mesh_get_aabb(RID p_mesh,RID p_skeleton) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,AABB());

	if (mesh->custom_aabb!=AABB())
		return mesh->custom_aabb;

	AABB aabb;

	for (int i=0;i<mesh->surfaces.size();i++) {

		if (i==0)
			aabb=mesh->surfaces[i]->aabb;
		else
			aabb.merge_with(mesh->surfaces[i]->aabb);
	}

	return aabb;
}

void RasterizerWIIU::mesh_set_custom_aabb(RID p_mesh,const AABB& p_aabb) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);

	mesh->custom_aabb=p_aabb;

}

AABB RasterizerWIIU::mesh_get_custom_aabb(RID p_mesh) const {

	const Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,AABB());

	return mesh->custom_aabb;
}


/* MULTIMESH API */

RID RasterizerWIIU::multimesh_create() {

	return multimesh_owner.make_rid( memnew( MultiMesh ));
}

void RasterizerWIIU::multimesh_set_instance_count(RID p_multimesh,int p_count) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);

	multimesh->elements.clear(); // make sure to delete everything, so it "fails" in all implementations
	multimesh->elements.resize(p_count);

}
int RasterizerWIIU::multimesh_get_instance_count(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,-1);

	return multimesh->elements.size();
}

void RasterizerWIIU::multimesh_set_mesh(RID p_multimesh,RID p_mesh) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);

	multimesh->mesh=p_mesh;

}
void RasterizerWIIU::multimesh_set_aabb(RID p_multimesh,const AABB& p_aabb) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	multimesh->aabb=p_aabb;
}
void RasterizerWIIU::multimesh_instance_set_transform(RID p_multimesh,int p_index,const Transform& p_transform) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_INDEX(p_index,multimesh->elements.size());
	MultiMesh::Element &e=multimesh->elements[p_index];

	e.matrix[0]=p_transform.basis.elements[0][0];
	e.matrix[1]=p_transform.basis.elements[1][0];
	e.matrix[2]=p_transform.basis.elements[2][0];
	e.matrix[3]=0;
	e.matrix[4]=p_transform.basis.elements[0][1];
	e.matrix[5]=p_transform.basis.elements[1][1];
	e.matrix[6]=p_transform.basis.elements[2][1];
	e.matrix[7]=0;
	e.matrix[8]=p_transform.basis.elements[0][2];
	e.matrix[9]=p_transform.basis.elements[1][2];
	e.matrix[10]=p_transform.basis.elements[2][2];
	e.matrix[11]=0;
	e.matrix[12]=p_transform.origin.x;
	e.matrix[13]=p_transform.origin.y;
	e.matrix[14]=p_transform.origin.z;
	e.matrix[15]=1;

}
void RasterizerWIIU::multimesh_instance_set_color(RID p_multimesh,int p_index,const Color& p_color) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh)
	ERR_FAIL_INDEX(p_index,multimesh->elements.size());
	MultiMesh::Element &e=multimesh->elements[p_index];
	e.color[0]=CLAMP(p_color.r*255,0,255);
	e.color[1]=CLAMP(p_color.g*255,0,255);
	e.color[2]=CLAMP(p_color.b*255,0,255);
	e.color[3]=CLAMP(p_color.a*255,0,255);


}

RID RasterizerWIIU::multimesh_get_mesh(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,RID());

	return multimesh->mesh;
}
AABB RasterizerWIIU::multimesh_get_aabb(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,AABB());

	return multimesh->aabb;
}

Transform RasterizerWIIU::multimesh_instance_get_transform(RID p_multimesh,int p_index) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,Transform());

	ERR_FAIL_INDEX_V(p_index,multimesh->elements.size(),Transform());
	MultiMesh::Element &e=multimesh->elements[p_index];

	Transform tr;

	tr.basis.elements[0][0]=e.matrix[0];
	tr.basis.elements[1][0]=e.matrix[1];
	tr.basis.elements[2][0]=e.matrix[2];
	tr.basis.elements[0][1]=e.matrix[4];
	tr.basis.elements[1][1]=e.matrix[5];
	tr.basis.elements[2][1]=e.matrix[6];
	tr.basis.elements[0][2]=e.matrix[8];
	tr.basis.elements[1][2]=e.matrix[9];
	tr.basis.elements[2][2]=e.matrix[10];
	tr.origin.x=e.matrix[12];
	tr.origin.y=e.matrix[13];
	tr.origin.z=e.matrix[14];

	return tr;
}
Color RasterizerWIIU::multimesh_instance_get_color(RID p_multimesh,int p_index) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,Color());
	ERR_FAIL_INDEX_V(p_index,multimesh->elements.size(),Color());
	MultiMesh::Element &e=multimesh->elements[p_index];
	Color c;
	c.r=e.color[0]/255.0;
	c.g=e.color[1]/255.0;
	c.b=e.color[2]/255.0;
	c.a=e.color[3]/255.0;

	return c;

}

void RasterizerWIIU::multimesh_set_visible_instances(RID p_multimesh,int p_visible) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	multimesh->visible=p_visible;

}

int RasterizerWIIU::multimesh_get_visible_instances(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,-1);
	return multimesh->visible;

}

/* IMMEDIATE API */


RID RasterizerWIIU::immediate_create() {

	Immediate *im = memnew( Immediate );
	return immediate_owner.make_rid(im);

}

void RasterizerWIIU::immediate_begin(RID p_immediate, VS::PrimitiveType p_rimitive, RID p_texture){


}
void RasterizerWIIU::immediate_vertex(RID p_immediate,const Vector3& p_vertex){


}
void RasterizerWIIU::immediate_normal(RID p_immediate,const Vector3& p_normal){


}
void RasterizerWIIU::immediate_tangent(RID p_immediate,const Plane& p_tangent){


}
void RasterizerWIIU::immediate_color(RID p_immediate,const Color& p_color){


}
void RasterizerWIIU::immediate_uv(RID p_immediate,const Vector2& tex_uv){


}
void RasterizerWIIU::immediate_uv2(RID p_immediate,const Vector2& tex_uv){


}

void RasterizerWIIU::immediate_end(RID p_immediate){


}
void RasterizerWIIU::immediate_clear(RID p_immediate) {


}

AABB RasterizerWIIU::immediate_get_aabb(RID p_immediate) const {

	return AABB(Vector3(-1,-1,-1),Vector3(2,2,2));
}

void RasterizerWIIU::immediate_set_material(RID p_immediate,RID p_material) {

	Immediate *im = immediate_owner.get(p_immediate);
	ERR_FAIL_COND(!im);
	im->material=p_material;
}

RID RasterizerWIIU::immediate_get_material(RID p_immediate) const {

	const Immediate *im = immediate_owner.get(p_immediate);
	ERR_FAIL_COND_V(!im,RID());
	return im->material;

}


/* PARTICLES API */

RID RasterizerWIIU::particles_create() {

	Particles *particles = memnew( Particles );
	ERR_FAIL_COND_V(!particles,RID());
	return particles_owner.make_rid(particles);
}

void RasterizerWIIU::particles_set_amount(RID p_particles, int p_amount) {

	ERR_FAIL_COND(p_amount<1);
	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.amount=p_amount;

}

int RasterizerWIIU::particles_get_amount(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.amount;

}

void RasterizerWIIU::particles_set_emitting(RID p_particles, bool p_emitting) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.emitting=p_emitting;;

}
bool RasterizerWIIU::particles_is_emitting(RID p_particles) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,false);
	return particles->data.emitting;

}

void RasterizerWIIU::particles_set_visibility_aabb(RID p_particles, const AABB& p_visibility) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.visibility_aabb=p_visibility;

}

void RasterizerWIIU::particles_set_emission_half_extents(RID p_particles, const Vector3& p_half_extents) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.emission_half_extents=p_half_extents;
}
Vector3 RasterizerWIIU::particles_get_emission_half_extents(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());

	return particles->data.emission_half_extents;
}

void RasterizerWIIU::particles_set_emission_base_velocity(RID p_particles, const Vector3& p_base_velocity) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.emission_base_velocity=p_base_velocity;
}

Vector3 RasterizerWIIU::particles_get_emission_base_velocity(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());

	return particles->data.emission_base_velocity;
}


void RasterizerWIIU::particles_set_emission_points(RID p_particles, const DVector<Vector3>& p_points) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.emission_points=p_points;
}

DVector<Vector3> RasterizerWIIU::particles_get_emission_points(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,DVector<Vector3>());

	return particles->data.emission_points;

}

void RasterizerWIIU::particles_set_gravity_normal(RID p_particles, const Vector3& p_normal) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.gravity_normal=p_normal;

}
Vector3 RasterizerWIIU::particles_get_gravity_normal(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());

	return particles->data.gravity_normal;
}


AABB RasterizerWIIU::particles_get_visibility_aabb(RID p_particles) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,AABB());
	return particles->data.visibility_aabb;

}

void RasterizerWIIU::particles_set_variable(RID p_particles, VS::ParticleVariable p_variable,float p_value) {

	ERR_FAIL_INDEX(p_variable,VS::PARTICLE_VAR_MAX);

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.particle_vars[p_variable]=p_value;

}
float RasterizerWIIU::particles_get_variable(RID p_particles, VS::ParticleVariable p_variable) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.particle_vars[p_variable];
}

void RasterizerWIIU::particles_set_randomness(RID p_particles, VS::ParticleVariable p_variable,float p_randomness) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.particle_randomness[p_variable]=p_randomness;

}
float RasterizerWIIU::particles_get_randomness(RID p_particles, VS::ParticleVariable p_variable) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.particle_randomness[p_variable];

}

void RasterizerWIIU::particles_set_color_phases(RID p_particles, int p_phases) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_COND( p_phases<0 || p_phases>VS::MAX_PARTICLE_COLOR_PHASES );
	particles->data.color_phase_count=p_phases;

}
int RasterizerWIIU::particles_get_color_phases(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.color_phase_count;
}


void RasterizerWIIU::particles_set_color_phase_pos(RID p_particles, int p_phase, float p_pos) {

	ERR_FAIL_INDEX(p_phase, VS::MAX_PARTICLE_COLOR_PHASES);
	if (p_pos<0.0)
		p_pos=0.0;
	if (p_pos>1.0)
		p_pos=1.0;

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.color_phases[p_phase].pos=p_pos;

}
float RasterizerWIIU::particles_get_color_phase_pos(RID p_particles, int p_phase) const {

	ERR_FAIL_INDEX_V(p_phase, VS::MAX_PARTICLE_COLOR_PHASES, -1.0);

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.color_phases[p_phase].pos;

}

void RasterizerWIIU::particles_set_color_phase_color(RID p_particles, int p_phase, const Color& p_color) {

	ERR_FAIL_INDEX(p_phase, VS::MAX_PARTICLE_COLOR_PHASES);
	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.color_phases[p_phase].color=p_color;

	//update alpha
	particles->has_alpha=false;
	for(int i=0;i<VS::MAX_PARTICLE_COLOR_PHASES;i++) {
		if (particles->data.color_phases[i].color.a<0.99)
			particles->has_alpha=true;
	}

}

Color RasterizerWIIU::particles_get_color_phase_color(RID p_particles, int p_phase) const {

	ERR_FAIL_INDEX_V(p_phase, VS::MAX_PARTICLE_COLOR_PHASES, Color());

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Color());
	return particles->data.color_phases[p_phase].color;

}

void RasterizerWIIU::particles_set_attractors(RID p_particles, int p_attractors) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_COND( p_attractors<0 || p_attractors>VisualServer::MAX_PARTICLE_ATTRACTORS );
	particles->data.attractor_count=p_attractors;

}
int RasterizerWIIU::particles_get_attractors(RID p_particles) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.attractor_count;
}

void RasterizerWIIU::particles_set_attractor_pos(RID p_particles, int p_attractor, const Vector3& p_pos) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_INDEX(p_attractor,particles->data.attractor_count);
	particles->data.attractors[p_attractor].pos=p_pos;;
}
Vector3 RasterizerWIIU::particles_get_attractor_pos(RID p_particles,int p_attractor) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());
	ERR_FAIL_INDEX_V(p_attractor,particles->data.attractor_count,Vector3());
	return particles->data.attractors[p_attractor].pos;
}

void RasterizerWIIU::particles_set_attractor_strength(RID p_particles, int p_attractor, float p_force) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_INDEX(p_attractor,particles->data.attractor_count);
	particles->data.attractors[p_attractor].force=p_force;
}

float RasterizerWIIU::particles_get_attractor_strength(RID p_particles,int p_attractor) const {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,0);
	ERR_FAIL_INDEX_V(p_attractor,particles->data.attractor_count,0);
	return particles->data.attractors[p_attractor].force;
}

void RasterizerWIIU::particles_set_material(RID p_particles, RID p_material,bool p_owned) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	if (particles->material_owned && particles->material.is_valid())
		free(particles->material);

	particles->material_owned=p_owned;

	particles->material=p_material;

}
RID RasterizerWIIU::particles_get_material(RID p_particles) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,RID());
	return particles->material;

}

void RasterizerWIIU::particles_set_use_local_coordinates(RID p_particles, bool p_enable) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.local_coordinates=p_enable;

}

bool RasterizerWIIU::particles_is_using_local_coordinates(RID p_particles) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,false);
	return particles->data.local_coordinates;
}
bool RasterizerWIIU::particles_has_height_from_velocity(RID p_particles) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,false);
	return particles->data.height_from_velocity;
}

void RasterizerWIIU::particles_set_height_from_velocity(RID p_particles, bool p_enable) {

	Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.height_from_velocity=p_enable;

}

AABB RasterizerWIIU::particles_get_aabb(RID p_particles) const {

	const Particles* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,AABB());
	return particles->data.visibility_aabb;
}

/* SKELETON API */

RID RasterizerWIIU::skeleton_create() {

	Skeleton *skeleton = memnew( Skeleton );
	ERR_FAIL_COND_V(!skeleton,RID());
	return skeleton_owner.make_rid( skeleton );
}

template <bool USE_NORMAL, bool USE_TANGENT, bool INPLACE>
void RasterizerWIIU::_skeleton_xform(const uint8_t *p_src_array, int p_src_stride, uint8_t *p_dst_array, int p_dst_stride, int p_elements, const uint8_t *p_src_bones, const uint8_t *p_src_weights, const Skeleton::Bone *p_bone_xforms) {

	uint32_t basesize = 3;
	if (USE_NORMAL)
		basesize += 3;
	if (USE_TANGENT)
		basesize += 4;

	uint32_t extra = (p_dst_stride - basesize * 4);
	const int dstvec_size = 3 + (USE_NORMAL ? 3 : 0) + (USE_TANGENT ? 4 : 0);
	float dstcopy[dstvec_size];

	for (int i = 0; i < p_elements; i++) {

		uint32_t ss = p_src_stride * i;
		uint32_t ds = p_dst_stride * i;
		const uint16_t *bi = (const uint16_t *)&p_src_bones[ss];
		const float *bw = (const float *)&p_src_weights[ss];
		const float *src_vec = (const float *)&p_src_array[ss];
		float *dst_vec;
		if (INPLACE)
			dst_vec = dstcopy;
		else
			dst_vec = (float *)&p_dst_array[ds];

		dst_vec[0] = 0.0;
		dst_vec[1] = 0.0;
		dst_vec[2] = 0.0;
		//conditionals simply removed by optimizer
		if (USE_NORMAL) {

			dst_vec[3] = 0.0;
			dst_vec[4] = 0.0;
			dst_vec[5] = 0.0;

			if (USE_TANGENT) {

				dst_vec[6] = 0.0;
				dst_vec[7] = 0.0;
				dst_vec[8] = 0.0;
				dst_vec[9] = src_vec[9];
			}
		} else {

			if (USE_TANGENT) {

				dst_vec[3] = 0.0;
				dst_vec[4] = 0.0;
				dst_vec[5] = 0.0;
				dst_vec[6] = src_vec[6];
			}
		}

#define _XFORM_BONE(m_idx)                                                                     \
	if (bw[m_idx] == 0)                                                                        \
		goto end;                                                                              \
	p_bone_xforms[bi[m_idx]].transform_add_mul3(&src_vec[0], &dst_vec[0], bw[m_idx]);          \
	if (USE_NORMAL) {                                                                          \
		p_bone_xforms[bi[m_idx]].transform3_add_mul3(&src_vec[3], &dst_vec[3], bw[m_idx]);     \
		if (USE_TANGENT) {                                                                     \
			p_bone_xforms[bi[m_idx]].transform3_add_mul3(&src_vec[6], &dst_vec[6], bw[m_idx]); \
		}                                                                                      \
	} else {                                                                                   \
		if (USE_TANGENT) {                                                                     \
			p_bone_xforms[bi[m_idx]].transform3_add_mul3(&src_vec[3], &dst_vec[3], bw[m_idx]); \
		}                                                                                      \
	}

		_XFORM_BONE(0);
		_XFORM_BONE(1);
		_XFORM_BONE(2);
		_XFORM_BONE(3);

	end:

		if (INPLACE) {

			const uint8_t *esp = (const uint8_t *)dstcopy;
			uint8_t *edp = (uint8_t *)&p_dst_array[ds];

			for (uint32_t j = 0; j < dstvec_size * 4; j++) {

				edp[j] = esp[j];
			}

		} else {
			//copy extra stuff
			const uint8_t *esp = (const uint8_t *)&src_vec[basesize];
			uint8_t *edp = (uint8_t *)&dst_vec[basesize];

			for (uint32_t j = 0; j < extra; j++) {

				edp[j] = esp[j];
			}
		}
	}
}

void RasterizerWIIU::skeleton_resize(RID p_skeleton,int p_bones) {

	Skeleton *skeleton = skeleton_owner.get(p_skeleton);
	ERR_FAIL_COND(!skeleton);
	if (p_bones == skeleton->bones.size()) {
		return;
	};
	if (use_hw_skeleton_xform) {

		if (next_power_of_2(p_bones) != next_power_of_2(skeleton->bones.size())) {
			if (skeleton->tex_id) {
				// glDeleteTextures(1, &skeleton->tex_id);
				skeleton->tex_id = 0;
			}

			if (p_bones) {

				// glGenTextures(1, &skeleton->tex_id);
// 				glActiveTexture(GL_TEXTURE0);
				// glBindTexture(GL_TEXTURE_2D, skeleton->tex_id);
				int ps = next_power_of_2(p_bones * 3);
#ifdef GLEW_ENABLED
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ps, 1, 0, GL_RGBA, GL_FLOAT, skel_default.ptr());
#else
				// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ps, 1, 0, GL_RGBA, GL_FLOAT, skel_default.ptr());
#endif
				// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				skeleton->pixel_size = 1.0 / ps;

				// glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		if (!skeleton->dirty_list.in_list()) {
			_skeleton_dirty_list.add(&skeleton->dirty_list);
		}
	}
	skeleton->bones.resize(p_bones);

}
int RasterizerWIIU::skeleton_get_bone_count(RID p_skeleton) const {

	Skeleton *skeleton = skeleton_owner.get(p_skeleton);
	ERR_FAIL_COND_V(!skeleton, -1);
	return skeleton->bones.size();
}
void RasterizerWIIU::skeleton_bone_set_transform(RID p_skeleton,int p_bone, const Transform& p_transform) {


	Skeleton *skeleton = skeleton_owner.get(p_skeleton);
	ERR_FAIL_COND(!skeleton);
	ERR_FAIL_INDEX(p_bone, skeleton->bones.size());

	Skeleton::Bone &b = skeleton->bones[p_bone];

	b.mtx[0][0] = p_transform.basis[0][0];
	b.mtx[0][1] = p_transform.basis[1][0];
	b.mtx[0][2] = p_transform.basis[2][0];
	b.mtx[1][0] = p_transform.basis[0][1];
	b.mtx[1][1] = p_transform.basis[1][1];
	b.mtx[1][2] = p_transform.basis[2][1];
	b.mtx[2][0] = p_transform.basis[0][2];
	b.mtx[2][1] = p_transform.basis[1][2];
	b.mtx[2][2] = p_transform.basis[2][2];
	b.mtx[3][0] = p_transform.origin[0];
	b.mtx[3][1] = p_transform.origin[1];
	b.mtx[3][2] = p_transform.origin[2];

	if (skeleton->tex_id) {
		if (!skeleton->dirty_list.in_list()) {
			_skeleton_dirty_list.add(&skeleton->dirty_list);
		}
	}
}

Transform RasterizerWIIU::skeleton_bone_get_transform(RID p_skeleton,int p_bone) {

	Skeleton *skeleton = skeleton_owner.get(p_skeleton);
	ERR_FAIL_COND_V(!skeleton, Transform());
	ERR_FAIL_INDEX_V(p_bone, skeleton->bones.size(), Transform());

	const Skeleton::Bone &b = skeleton->bones[p_bone];

	Transform t;
	t.basis[0][0] = b.mtx[0][0];
	t.basis[1][0] = b.mtx[0][1];
	t.basis[2][0] = b.mtx[0][2];
	t.basis[0][1] = b.mtx[1][0];
	t.basis[1][1] = b.mtx[1][1];
	t.basis[2][1] = b.mtx[1][2];
	t.basis[0][2] = b.mtx[2][0];
	t.basis[1][2] = b.mtx[2][1];
	t.basis[2][2] = b.mtx[2][2];
	t.origin[0] = b.mtx[3][0];
	t.origin[1] = b.mtx[3][1];
	t.origin[2] = b.mtx[3][2];

	return t;
}


/* LIGHT API */

RID RasterizerWIIU::light_create(VS::LightType p_type) {

	Light *light = memnew( Light );
	light->type=p_type;
	return light_owner.make_rid(light);
}

VS::LightType RasterizerWIIU::light_get_type(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,VS::LIGHT_OMNI);
	return light->type;
}

void RasterizerWIIU::light_set_color(RID p_light,VS::LightColor p_type, const Color& p_color) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	ERR_FAIL_INDEX( p_type, 3 );
	light->colors[p_type]=p_color;
}
Color RasterizerWIIU::light_get_color(RID p_light,VS::LightColor p_type) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light, Color());
	ERR_FAIL_INDEX_V( p_type, 3, Color() );
	return light->colors[p_type];
}

void RasterizerWIIU::light_set_shadow(RID p_light,bool p_enabled) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	light->shadow_enabled=p_enabled;
}

bool RasterizerWIIU::light_has_shadow(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,false);
	return light->shadow_enabled;
}

void RasterizerWIIU::light_set_volumetric(RID p_light,bool p_enabled) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	light->volumetric_enabled=p_enabled;

}
bool RasterizerWIIU::light_is_volumetric(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,false);
	return light->volumetric_enabled;
}

void RasterizerWIIU::light_set_projector(RID p_light,RID p_texture) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	light->projector=p_texture;
}
RID RasterizerWIIU::light_get_projector(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,RID());
	return light->projector;
}

void RasterizerWIIU::light_set_var(RID p_light, VS::LightParam p_var, float p_value) {

	Light * light = light_owner.get( p_light );
	ERR_FAIL_COND(!light);
	ERR_FAIL_INDEX( p_var, VS::LIGHT_PARAM_MAX );

	light->vars[p_var]=p_value;
}
float RasterizerWIIU::light_get_var(RID p_light, VS::LightParam p_var) const {

	Light * light = light_owner.get( p_light );
	ERR_FAIL_COND_V(!light,0);

	ERR_FAIL_INDEX_V( p_var, VS::LIGHT_PARAM_MAX,0 );

	return light->vars[p_var];
}

void RasterizerWIIU::light_set_operator(RID p_light,VS::LightOp p_op) {

	Light * light = light_owner.get( p_light );
	ERR_FAIL_COND(!light);


};

VS::LightOp RasterizerWIIU::light_get_operator(RID p_light) const {

	return VS::LightOp(0);
};

void RasterizerWIIU::light_omni_set_shadow_mode(RID p_light,VS::LightOmniShadowMode p_mode) {


}

VS::LightOmniShadowMode RasterizerWIIU::light_omni_get_shadow_mode(RID p_light) const{

	return VS::LightOmniShadowMode(0);
}

void RasterizerWIIU::light_directional_set_shadow_mode(RID p_light,VS::LightDirectionalShadowMode p_mode) {


}

VS::LightDirectionalShadowMode RasterizerWIIU::light_directional_get_shadow_mode(RID p_light) const {

	return VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL;
}

void RasterizerWIIU::light_directional_set_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param, float p_value) {


}

float RasterizerWIIU::light_directional_get_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param) const {

	return 0;
}


AABB RasterizerWIIU::light_get_aabb(RID p_light) const {

	Light *light = light_owner.get( p_light );
	ERR_FAIL_COND_V(!light,AABB());

	switch( light->type ) {

		case VS::LIGHT_SPOT: {

			float len=light->vars[VS::LIGHT_PARAM_RADIUS];
			float size=Math::tan(Math::deg2rad(light->vars[VS::LIGHT_PARAM_SPOT_ANGLE]))*len;
			return AABB( Vector3( -size,-size,-len ), Vector3( size*2, size*2, len ) );
		} break;
		case VS::LIGHT_OMNI: {

			float r = light->vars[VS::LIGHT_PARAM_RADIUS];
			return AABB( -Vector3(r,r,r), Vector3(r,r,r)*2 );
		} break;
		case VS::LIGHT_DIRECTIONAL: {

			return AABB();
		} break;
		default: {}
	}

	ERR_FAIL_V( AABB() );
}


RID RasterizerWIIU::light_instance_create(RID p_light) {

	Light *light = light_owner.get( p_light );
	ERR_FAIL_COND_V(!light, RID());

	LightInstance *light_instance = memnew( LightInstance );

	light_instance->light=p_light;
	light_instance->base=light;
	light_instance->last_pass=0;

	return light_instance_owner.make_rid( light_instance );
}
void RasterizerWIIU::light_instance_set_transform(RID p_light_instance,const Transform& p_transform) {

	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND(!lighti);
	lighti->transform=p_transform;

}

bool RasterizerWIIU::light_instance_has_shadow(RID p_light_instance) const {

	return false;

	/*
	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND_V(!lighti, false);

	if (!lighti->base->shadow_enabled)
		return false;

	if (lighti->base->type==VS::LIGHT_DIRECTIONAL) {
		if (lighti->shadow_pass!=scene_pass)
			return false;

	} else {
		if (lighti->shadow_pass!=frame)
			return false;
	}*/



	//return !lighti->shadow_buffers.empty();

}


bool RasterizerWIIU::light_instance_assign_shadow(RID p_light_instance) {

	return false;

}


Rasterizer::ShadowType RasterizerWIIU::light_instance_get_shadow_type(RID p_light_instance) const {

	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND_V(!lighti,Rasterizer::SHADOW_NONE);

	switch(lighti->base->type) {

		case VS::LIGHT_DIRECTIONAL: return SHADOW_PSM; break;
		case VS::LIGHT_OMNI: return SHADOW_DUAL_PARABOLOID; break;
		case VS::LIGHT_SPOT: return SHADOW_SIMPLE; break;
	}

	return Rasterizer::SHADOW_NONE;
}

Rasterizer::ShadowType RasterizerWIIU::light_instance_get_shadow_type(RID p_light_instance,bool p_far) const {

	return SHADOW_NONE;
}
void RasterizerWIIU::light_instance_set_shadow_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform& p_transform, float p_split_near,float p_split_far) {

	LightInstance *lighti = light_instance_owner.get(p_light_instance);
	ERR_FAIL_COND(!lighti);

	ERR_FAIL_COND(lighti->base->type != VS::LIGHT_DIRECTIONAL);
	//	ERR_FAIL_INDEX(p_index,1);

	lighti->custom_projection[p_index] = p_camera;
	lighti->custom_transform[p_index] = p_transform;
	lighti->shadow_split[p_index] = 1.0 / p_split_far;
}

int RasterizerWIIU::light_instance_get_shadow_passes(RID p_light_instance) const {

	return 0;
}

bool RasterizerWIIU::light_instance_get_pssm_shadow_overlap(RID p_light_instance) const {

	return false;
}

void RasterizerWIIU::light_instance_set_custom_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform& p_transform, float p_split_near,float p_split_far) {

	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND(!lighti);

	ERR_FAIL_COND(lighti->base->type!=VS::LIGHT_DIRECTIONAL);
	ERR_FAIL_INDEX(p_index,1);

// 	lighti->custom_projection=p_camera;
// 	lighti->custom_transform=p_transform;

}
void RasterizerWIIU::shadow_clear_near() {


}

bool RasterizerWIIU::shadow_allocate_near(RID p_light) {

	return false;
}

bool RasterizerWIIU::shadow_allocate_far(RID p_light) {

	return false;
}

/* PARTICLES INSTANCE */

RID RasterizerWIIU::particles_instance_create(RID p_particles) {

	ERR_FAIL_COND_V(!particles_owner.owns(p_particles),RID());
	ParticlesInstance *particles_instance = memnew( ParticlesInstance );
	ERR_FAIL_COND_V(!particles_instance, RID() );
	particles_instance->particles=p_particles;
	return particles_instance_owner.make_rid(particles_instance);
}

void RasterizerWIIU::particles_instance_set_transform(RID p_particles_instance,const Transform& p_transform) {

	ParticlesInstance *particles_instance=particles_instance_owner.get(p_particles_instance);
	ERR_FAIL_COND(!particles_instance);
	particles_instance->transform=p_transform;
}


/* RENDER API */
/* all calls (inside begin/end shadow) are always warranted to be in the following order: */


RID RasterizerWIIU::viewport_data_create() {

	return RID();
}

RID RasterizerWIIU::render_target_create(){

	return RID();

}
void RasterizerWIIU::render_target_set_size(RID p_render_target, int p_width, int p_height){


}
RID RasterizerWIIU::render_target_get_texture(RID p_render_target) const{

	return RID();

}
bool RasterizerWIIU::render_target_renedered_in_frame(RID p_render_target){

	return false;
}


void RasterizerWIIU::begin_frame() {


	window_size = Size2( OS::get_singleton()->get_video_mode().width, OS::get_singleton()->get_video_mode().height );
	print_line("begin frame - winsize: "+window_size);

	double time = (OS::get_singleton()->get_ticks_usec()/1000); // get msec
	time/=1000.0; // make secs
	time_delta=time-last_time;
	last_time=time;
	frame++;
	clear_viewport(Color(1,0,0));

	_rinfo.vertex_count=0;
	_rinfo.object_count=0;
	_rinfo.mat_change_count=0;
	_rinfo.shader_change_count=0;

	while (_skeleton_dirty_list.first()) {

		Skeleton *s = _skeleton_dirty_list.first()->self();

		float *sk_float = (float *)skinned_buffer;
		for (int i = 0; i < s->bones.size(); i++) {

			float *m = &sk_float[i * 12];
			const Skeleton::Bone &b = s->bones[i];
			m[0] = b.mtx[0][0];
			m[1] = b.mtx[1][0];
			m[2] = b.mtx[2][0];
			m[3] = b.mtx[3][0];

			m[4] = b.mtx[0][1];
			m[5] = b.mtx[1][1];
			m[6] = b.mtx[2][1];
			m[7] = b.mtx[3][1];

			m[8] = b.mtx[0][2];
			m[9] = b.mtx[1][2];
			m[10] = b.mtx[2][2];
			m[11] = b.mtx[3][2];
		}

// 		glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, s->tex_id);
		// glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, next_power_of_2(s->bones.size() * 3), 1, GL_RGBA, GL_FLOAT, sk_float);
		_skeleton_dirty_list.remove(_skeleton_dirty_list.first());
	}

	WHBGfxBeginRender();

	WHBGfxBeginRenderTV();

//	material_shader.set_uniform_default(MaterialShaderGLES1::SCREENZ_SCALE, Math::fmod(time, 3600.0));
	/* nehe ?*/

//	glClearColor(0,0,1,1);
//	glClear(GL_COLOR_BUFFER_BIT); //should not clear if anything else cleared..

	GX2SetFetchShader(&shaderGroup.fetchShader);
	GX2SetVertexShader(shaderGroup.vertexShader);
	GX2SetPixelShader(shaderGroup.pixelShader);
	GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);
}

void RasterizerWIIU::capture_viewport(Image* r_capture) {


}


void RasterizerWIIU::clear_viewport(const Color& p_color) {

	// GX2SetScissor( viewport.x, window_size.height-(viewport.height+viewport.y), viewport.width,viewport.height );
	// glEnable(GL_SCISSOR_TEST);
	WHBGfxClearColor(p_color.r,p_color.g,p_color.b,p_color.a);
	// glClear(GL_COLOR_BUFFER_BIT); //should not clear if anything else cleared..
	// glDisable(GL_SCISSOR_TEST);

};

void RasterizerWIIU::set_viewport(const VS::ViewportRect& p_viewport) {



	viewport=p_viewport;
	print_line("viewport: "+itos(p_viewport.x)+","+itos(p_viewport.y)+","+itos(p_viewport.width)+","+itos(p_viewport.height));

	// GX2SetViewport( viewport.x, window_size.height-(viewport.height+viewport.y), viewport.width,viewport.height, 0.0f, 1.0f );
}

void RasterizerWIIU::set_render_target(RID p_render_target, bool p_transparent_bg, bool p_vflip) {


}

void RasterizerWIIU::set_camera(const Transform &p_world, const CameraMatrix &p_projection, bool p_ortho_hint) {

	camera_transform = p_world;

	camera_transform_inverse = camera_transform.inverse();
	camera_projection = p_projection;
	camera_plane = Plane(camera_transform.origin, -camera_transform.basis.get_axis(2));
	camera_z_near = camera_projection.get_z_near();
	camera_z_far = camera_projection.get_z_far();
	camera_projection.get_viewport_size(camera_vp_size.x, camera_vp_size.y);
// 	camera_ortho = p_ortho_hint;
}

void RasterizerWIIU::begin_scene(RID p_viewport_data,RID p_env,VS::ScenarioDebugMode p_debug) {


	opaque_render_list.clear();
	alpha_render_list.clear();
	light_instance_count=0;
	scene_fx = NULL; // p_env.is_valid() ? fx_owner.get(p_env) : NULL;
	scene_pass++;
	last_light_id=0;
	current_env = p_env.is_valid() ? environment_owner.get(p_env) : NULL;
	directional_light_count=0;


	//set state

	// glCullFace(GL_FRONT);
	cull_front=true;
};

void RasterizerWIIU::begin_shadow_map( RID p_light_instance, int p_shadow_pass ) {
	ERR_FAIL_COND(shadow);
	shadow = light_instance_owner.get(p_light_instance);
	shadow_pass = p_shadow_pass;
	ERR_FAIL_COND(!shadow);

	opaque_render_list.clear();
	alpha_render_list.clear();
	//	pre_zpass_render_list.clear();
	light_instance_count = 0;

	// glCullFace(GL_FRONT);
	cull_front = true;
}

void RasterizerWIIU::set_camera(const Transform& p_world,const CameraMatrix& p_projection) {

	camera_transform=p_world;
	camera_transform_inverse=camera_transform.inverse();
	camera_projection=p_projection;
	camera_plane = Plane( camera_transform.origin, camera_transform.basis.get_axis(2) );
	camera_z_near=camera_projection.get_z_near();
	camera_z_far=camera_projection.get_z_far();
	camera_projection.get_viewport_size(camera_vp_size.x,camera_vp_size.y);
}

void RasterizerWIIU::add_light( RID p_light_instance ) {

#define LIGHT_FADE_TRESHOLD 0.05

	ERR_FAIL_COND( light_instance_count >= MAX_SCENE_LIGHTS );

	LightInstance *li = light_instance_owner.get(p_light_instance);
	ERR_FAIL_COND(!li);


	/* make light hash */

	// actually, not really a hash, but helps to sort the lights
	// and avoid recompiling redudant shader versions


	li->last_pass=scene_pass;
	li->sort_key=light_instance_count;

	 switch(li->base->type) {

		case VisualServer::LIGHT_DIRECTIONAL: {

			li->light_vector = camera_transform_inverse.basis.xform(li->transform.basis.get_axis(2)).normalized();
			if (directional_light_count<MAX_HW_LIGHTS) {

				directional_lights[directional_light_count++]=li;
			}

		} break;
		case VisualServer::LIGHT_OMNI: {

			  float radius = li->base->vars[VisualServer::LIGHT_PARAM_RADIUS];
			  if (radius==0)
				  radius=0.0001;
			  li->linear_att=(1/LIGHT_FADE_TRESHOLD)/radius;
			  li->light_vector = camera_transform_inverse.xform(li->transform.origin);

		} break;
		case VisualServer::LIGHT_SPOT: {

			float radius = li->base->vars[VisualServer::LIGHT_PARAM_RADIUS];
			if (radius==0)
				radius=0.0001;
			li->linear_att=(1/LIGHT_FADE_TRESHOLD)/radius;
			li->light_vector = camera_transform_inverse.xform(li->transform.origin);
			li->spot_vector = -camera_transform_inverse.basis.xform(li->transform.basis.get_axis(2)).normalized();
			//li->sort_key|=LIGHT_SPOT_BIT; // this way, omnis go first, spots go last and less shader versions are generated

			/*
			if (li->base->projector.is_valid()) {

				float far = li->base->vars[ VS::LIGHT_VAR_RADIUS ];
				ERR_FAIL_COND( far<=0 );
				float near= far/200.0;
				if (near<0.05)
					near=0.05;

				float angle = li->base->vars[ VS::LIGHT_VAR_SPOT_ANGLE ];

				//CameraMatrix proj;
				//proj.set_perspective( angle*2.0, 1.0, near, far );

				//Transform modelview=Transform(camera_transform_inverse * li->transform).inverse();
				//li->projector_mtx= proj * modelview;

			}*/
		} break;
	 }

	light_instances[light_instance_count++]=li;

}

void RasterizerWIIU::_add_geometry( const Geometry* p_geometry, const InstanceData *p_instance, const Geometry *p_geometry_cmp, const GeometryOwner *p_owner) {

	Material *m=NULL;
	RID m_src=p_instance->material_override.is_valid() ? p_instance->material_override : p_geometry->material;

	if (m_src)
		m=material_owner.get( m_src );

	if (!m) {
		m=material_owner.get( default_material );
	}

	ERR_FAIL_COND(!m);


	if (m->last_pass!=frame) {

		m->last_pass=frame;
	}


	LightInstance *lights[RenderList::MAX_LIGHTS];
	int light_count=0;

	RenderList *render_list=&opaque_render_list;
	if (m->fixed_flags[VS::FIXED_MATERIAL_FLAG_USE_ALPHA] || m->blend_mode!=VS::MATERIAL_BLEND_MODE_MIX) {
		render_list = &alpha_render_list;
	};

	if (!m->flags[VS::MATERIAL_FLAG_UNSHADED]) {

		int lis = p_instance->light_instances.size();

		for(int i=0;i<lis;i++) {
			if (light_count>=RenderList::MAX_LIGHTS)
				break;

			LightInstance *li=light_instance_owner.get( p_instance->light_instances[i] );

			if (!li || li->last_pass!=scene_pass) //lit by light not in visible scene
				continue;
			lights[light_count++]=li;
		}
	}

	RenderList::Element *e = render_list->add_element();

	e->geometry=p_geometry;
//	e->geometry_cmp=p_geometry_cmp;
	e->material=m;
	e->instance=p_instance;
	//e->depth=camera_plane.distance_to(p_world->origin);
	e->depth=camera_transform.origin.distance_to(p_instance->transform.origin);
	e->owner=p_owner;
	if (p_instance->skeleton.is_valid())
		e->skeleton=skeleton_owner.get(p_instance->skeleton);
	else
		e->skeleton=NULL;
	e->mirror=p_instance->mirror;
	if (m->flags[VS::MATERIAL_FLAG_INVERT_FACES])
		e->mirror=!e->mirror;

	e->light_key=0;
	e->light_count=0;


	if (!shadow) {


		if (m->flags[VS::MATERIAL_FLAG_UNSHADED]) {


			e->light_key--; //special key for all the shadeless people
		} else if (light_count) {

			for(int i=0;i<light_count;i++) {

				e->lights[i]=lights[i]->sort_key;
			}

			e->light_count=light_count;
			int poslight_count=light_count;
			if (poslight_count>1) {
				SortArray<uint16_t> light_sort;
				light_sort.sort(&e->lights[0],poslight_count); //generate an equal sort key
			}
		}

	}

}


void RasterizerWIIU::add_mesh( const RID& p_mesh, const InstanceData *p_data) {

	Mesh *mesh = mesh_owner.get(p_mesh);
	ERR_FAIL_COND(!mesh);

	int ssize = mesh->surfaces.size();

	for (int i=0;i<ssize;i++) {

		Surface *s = mesh->surfaces[i];
		_add_geometry(s,p_data,s,NULL);
	}

	mesh->last_pass=frame;

}

void RasterizerWIIU::add_multimesh( const RID& p_multimesh, const InstanceData *p_data){

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);

	if (!multimesh->mesh.is_valid())
		return;
	if (multimesh->elements.empty())
		return;

	Mesh *mesh = mesh_owner.get(multimesh->mesh);
	ERR_FAIL_COND(!mesh);

	int surf_count = mesh->surfaces.size();
	if (multimesh->last_pass!=scene_pass) {

		multimesh->cache_surfaces.resize(surf_count);
		for(int i=0;i<surf_count;i++) {

			multimesh->cache_surfaces[i].material=mesh->surfaces[i]->material;
			multimesh->cache_surfaces[i].has_alpha=mesh->surfaces[i]->has_alpha;
			multimesh->cache_surfaces[i].surface=mesh->surfaces[i];
		}

		multimesh->last_pass=scene_pass;
	}

	for(int i=0;i<surf_count;i++) {

		_add_geometry(&multimesh->cache_surfaces[i],p_data,multimesh->cache_surfaces[i].surface,multimesh);
	}


}

void RasterizerWIIU::add_particles( const RID& p_particle_instance, const InstanceData *p_data){

	//print_line("adding particles");
	ParticlesInstance *particles_instance = particles_instance_owner.get(p_particle_instance);
	ERR_FAIL_COND(!particles_instance);
	Particles *p=particles_owner.get( particles_instance->particles );
	ERR_FAIL_COND(!p);

	_add_geometry(p,p_data,p,particles_instance);

}


void RasterizerWIIU::_set_cull(bool p_front,bool p_reverse_cull) {

	bool front = p_front;
	if (p_reverse_cull)
		front=!front;

	if (front!=cull_front) {

		// glCullFace(front?GL_FRONT:GL_BACK);
		cull_front=front;
	}
}


void RasterizerWIIU::_setup_fixed_material(const Geometry *p_geometry,const Material *p_material) {

	if (!shadow) {

		///ambient @TODO offer global ambient group option

		//GLenum side = use_shaders?GL_FRONT:GL_FRONT_AND_BACK;
		GLenum side = GL_FRONT_AND_BACK;


		///diffuse
		Color diffuse_color=p_material->parameters[VS::FIXED_MATERIAL_PARAM_DIFFUSE];
		float diffuse_rgba[4]={
			diffuse_color.r,
			 diffuse_color.g,
			  diffuse_color.b,
			   diffuse_color.a
		};

		//color array overrides this
		// glColor4f( diffuse_rgba[0],diffuse_rgba[1],diffuse_rgba[2],diffuse_rgba[3]);
		last_color=diffuse_color;
		// glMaterialfv(side,GL_AMBIENT,diffuse_rgba);
		// glMaterialfv(side,GL_DIFFUSE,diffuse_rgba);
		//specular

		const Color specular_color=p_material->parameters[VS::FIXED_MATERIAL_PARAM_SPECULAR];
		float specular_rgba[4]={
			specular_color.r,
			specular_color.g,
			specular_color.b,
			1.0
		};

		// glMaterialfv(side,GL_SPECULAR,specular_rgba);

		const Color emission=p_material->parameters[VS::FIXED_MATERIAL_PARAM_EMISSION];


		float emission_rgba[4]={
			emission.r,
			emission.g,
			emission.b,
			1.0 //p_material->parameters[VS::FIXED_MATERIAL_PARAM_DETAIL_MIX]
		};

		// glMaterialfv(side,GL_EMISSION,emission_rgba);

		// glMaterialf(side,GL_SHININESS,p_material->parameters[VS::FIXED_MATERIAL_PARAM_SPECULAR_EXP]);

		Plane sparams=p_material->parameters[VS::FIXED_MATERIAL_PARAM_SHADE_PARAM];
		//depth test?


	}


	if (p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE].is_valid()) {

		Texture *texture = texture_owner.get( p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE] );
		ERR_FAIL_COND(!texture);
		// glEnable(GL_TEXTURE_2D);
		//glActiveTexture(GL_TEXTURE0);
		// glBindTexture( GL_TEXTURE_2D,texture->tex_id );
	} else {

		// glDisable(GL_TEXTURE_2D);
	}

}

void RasterizerWIIU::_setup_material(const Geometry *p_geometry,const Material *p_material) {

	// if (p_material->flags[VS::MATERIAL_FLAG_DOUBLE_SIDED])
		// glEnable(GL_CULL_FACE);
	// else {
		// glDisable(GL_CULL_FACE);
	// }

/*	if (p_material->flags[VS::MATERIAL_FLAG_WIREFRAME])
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);*/

	// if (p_material->line_width > 0)
		// glLineWidth(p_material->line_width);

	if (!shadow) {


		if (blend_mode!=p_material->blend_mode) {
			switch(p_material->blend_mode) {


				 case VS::MATERIAL_BLEND_MODE_MIX: {
					////glBlendEquation(GL_FUNC_ADD);
					// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

				 } break;
				 case VS::MATERIAL_BLEND_MODE_ADD: {

					////glBlendEquation(GL_FUNC_ADD);
					// glBlendFunc(GL_SRC_ALPHA,GL_ONE);

				 } break;
				 case VS::MATERIAL_BLEND_MODE_SUB: {

					////glBlendEquation(GL_FUNC_SUBTRACT);
					// glBlendFunc(GL_SRC_ALPHA,GL_ONE);
				 } break;
				case VS::MATERIAL_BLEND_MODE_MUL: {
					////glBlendEquation(GL_FUNC_ADD);
					// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

				} break;

			}
			blend_mode=p_material->blend_mode;
		}

		if (lighting!=!p_material->flags[VS::MATERIAL_FLAG_UNSHADED]) {
			if (p_material->flags[VS::MATERIAL_FLAG_UNSHADED]) {
				// glDisable(GL_LIGHTING);
			} else {
				// glEnable(GL_LIGHTING);
			}
			lighting=!p_material->flags[VS::MATERIAL_FLAG_UNSHADED];
		}

	}

	bool current_depth_write=p_material->depth_draw_mode!=VS::MATERIAL_DEPTH_DRAW_ALWAYS; //broken
	bool current_depth_test=!p_material->flags[VS::MATERIAL_FLAG_ONTOP];


	_setup_fixed_material(p_geometry,p_material);

	if (current_depth_write!=depth_write) {

		depth_write=current_depth_write;
		// glDepthMask(depth_write);
	}

	if (current_depth_test!=depth_test) {

		depth_test=current_depth_test;/*
		if (depth_test)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);*/
	}
}
/*
static const MaterialShaderGLES1::Conditionals _gl_light_version[4][3]={
	{MaterialShaderGLES1::LIGHT_0_DIRECTIONAL,MaterialShaderGLES1::LIGHT_0_OMNI,MaterialShaderGLES1::LIGHT_0_SPOT},
	{MaterialShaderGLES1::LIGHT_1_DIRECTIONAL,MaterialShaderGLES1::LIGHT_1_OMNI,MaterialShaderGLES1::LIGHT_1_SPOT},
	{MaterialShaderGLES1::LIGHT_2_DIRECTIONAL,MaterialShaderGLES1::LIGHT_2_OMNI,MaterialShaderGLES1::LIGHT_2_SPOT},
	{MaterialShaderGLES1::LIGHT_3_DIRECTIONAL,MaterialShaderGLES1::LIGHT_3_OMNI,MaterialShaderGLES1::LIGHT_3_SPOT}
};

static const MaterialShaderGLES1::Conditionals _gl_light_shadow[4]={
	MaterialShaderGLES1::LIGHT_0_SHADOW,
	MaterialShaderGLES1::LIGHT_1_SHADOW,
	MaterialShaderGLES1::LIGHT_2_SHADOW,
	MaterialShaderGLES1::LIGHT_3_SHADOW
};
*/


void RasterizerWIIU::_setup_light(LightInstance* p_instance, int p_idx) {

	Light* ld = p_instance->base;

//	material_shader.set_conditional(MaterialShaderGLES1::LIGHT_0_DIRECTIONAL, true);

	//material_shader.set_uniform_default(MaterialShaderGLES1::LIGHT_0_DIFFUSE, ld->colors[VS::LIGHT_COLOR_DIFFUSE]);
	//material_shader.set_uniform_default(MaterialShaderGLES1::LIGHT_0_SPECULAR, ld->colors[VS::LIGHT_COLOR_SPECULAR]);
	//material_shader.set_uniform_default(MaterialShaderGLES1::LIGHT_0_AMBIENT, ld->colors[VS::LIGHT_COLOR_AMBIENT]);

	GLenum glid = GL_LIGHT0+p_idx;

	Color diff_color = ld->colors[VS::LIGHT_COLOR_DIFFUSE];
	float emult = ld->vars[VS::LIGHT_PARAM_ENERGY];

	if (ld->type!=VS::LIGHT_DIRECTIONAL)
		emult*=4.0;

	GLfloat diffuse_sdark[4]={
		diff_color.r*emult,
		diff_color.g*emult,
		diff_color.b*emult,
		1.0
	};

	// glLightfv(glid , GL_DIFFUSE, diffuse_sdark);

	Color amb_color = Color(0,0,0);
	GLfloat amb_stexsize[4]={
		amb_color.r,
		amb_color.g,
		amb_color.b,
		1.0
	};

	// glLightfv(glid , GL_AMBIENT, amb_stexsize );

	Color spec_color = ld->colors[VS::LIGHT_COLOR_SPECULAR];
	GLfloat spec_op[4]={
		spec_color.r,
		spec_color.g,
		spec_color.b,
		1.0
	};

	// glLightfv(glid , GL_SPECULAR, spec_op );

	switch(ld->type) {

		case VS::LIGHT_DIRECTIONAL: {

			// glMatrixMode(GL_MODELVIEW);
			// glPushMatrix();
			// glLoadIdentity();

			// glLightf(glid,GL_CONSTANT_ATTENUATION, 1);
			// glLightf(glid,GL_LINEAR_ATTENUATION, 0);
			// glLightf(glid,GL_QUADRATIC_ATTENUATION,0); // energy

			float lightdir[4]={
				p_instance->light_vector.x,
				p_instance->light_vector.y,
				p_instance->light_vector.z,
				0.0
			};

			// glLightfv(glid,GL_POSITION,lightdir); //at modelview
			// glLightf(glid,GL_SPOT_CUTOFF,180.0);
			// glLightf(glid,GL_SPOT_EXPONENT, 0);

			float sdir[4]={
				0,
				0,
				-1,
				0
			};

			// glLightfv(glid,GL_SPOT_DIRECTION,sdir); //at modelview

//			material_shader.set_uniform_default(MaterialShaderGLES1::LIGHT_0_DIRECTION, p_instance->light_vector);
			// glPopMatrix();

		} break;

		case VS::LIGHT_OMNI: {


			// glLightf(glid,GL_SPOT_CUTOFF,180.0);
			// glLightf(glid,GL_SPOT_EXPONENT, 0);


			// glLightf(glid,GL_CONSTANT_ATTENUATION, 0);
			// glLightf(glid,GL_LINEAR_ATTENUATION, p_instance->linear_att);
			// glLightf(glid,GL_QUADRATIC_ATTENUATION, 0); // wut?

			// glMatrixMode(GL_MODELVIEW);
			// glPushMatrix();
			// glLoadIdentity();
			float lightpos[4]={
				p_instance->light_vector.x,
				p_instance->light_vector.y,
				p_instance->light_vector.z,
				1.0
			};

			// glLightfv(glid,GL_POSITION,lightpos); //at modelview

			// glPopMatrix();


		} break;
		case VS::LIGHT_SPOT: {

			// glLightf(glid,GL_SPOT_CUTOFF, ld->vars[VS::LIGHT_PARAM_SPOT_ANGLE]);
			// glLightf(glid,GL_SPOT_EXPONENT, ld->vars[VS::LIGHT_PARAM_SPOT_ATTENUATION]);


			// glLightf(glid,GL_CONSTANT_ATTENUATION, 0);
			// glLightf(glid,GL_LINEAR_ATTENUATION, p_instance->linear_att);
			// glLightf(glid,GL_QUADRATIC_ATTENUATION, 0); // wut?


			// glMatrixMode(GL_MODELVIEW);
			// glPushMatrix();
			// glLoadIdentity();
			float lightpos[4]={
				p_instance->light_vector.x,
				p_instance->light_vector.y,
				p_instance->light_vector.z,
				1.0
			};

			// glLightfv(glid,GL_POSITION,lightpos); //at modelview

			float lightdir[4]={
				p_instance->spot_vector.x,
				p_instance->spot_vector.y,
				p_instance->spot_vector.z,
				1.0
			};

			// glLightfv(glid,GL_SPOT_DIRECTION,lightdir); //at modelview

			// glPopMatrix();



		} break;

		default: break;
	}
};





void RasterizerWIIU::_setup_lights(const uint16_t * p_lights,int p_light_count) {

	if (shadow)
		return;



	for (int i=directional_light_count; i<MAX_HW_LIGHTS; i++) {

		if (i<(directional_light_count+p_light_count)) {


			// glEnable(GL_LIGHT0 + i);
			_setup_light(light_instances[p_lights[i]], i);

		} else {
			// glDisable(GL_LIGHT0 + i);

		}
	}

}



static const GLenum gl_client_states[] = {

	GL_VERTEX_ARRAY,
	GL_NORMAL_ARRAY,
	0, // ARRAY_TANGENT
	0,//GL_COLOR_ARRAY,
	GL_TEXTURE_COORD_ARRAY, // ARRAY_TEX_UV
	0,//GL_TEXTURE_COORD_ARRAY, // ARRAY_TEX_UV2
	0, // ARRAY_BONES
	0, // ARRAY_WEIGHTS
};

static const int gl_texcoord_index[VS::ARRAY_MAX-1] = {

	-1,
	-1,
	-1, // ARRAY_TANGENT
	-1,
	0, // ARRAY_TEX_UV
	-1,//1, // ARRAY_TEX_UV2
	-1, // ARRAY_BONES
	-1, // ARRAY_WEIGHTS
};


Error RasterizerWIIU::_setup_geometry(const Geometry *p_geometry, const Material* p_material, const Skeleton *p_skeleton,const float *p_morphs) {


	switch(p_geometry->type) {

		case Geometry::GEOMETRY_MULTISURFACE:
		case Geometry::GEOMETRY_SURFACE: {



			const Surface *surf=NULL;
			if (p_geometry->type==Geometry::GEOMETRY_SURFACE)
				surf=static_cast<const Surface*>(p_geometry);
			else if (p_geometry->type==Geometry::GEOMETRY_MULTISURFACE)
				surf=static_cast<const MultiMeshSurface*>(p_geometry)->surface;


			if (surf->format != surf->configured_format) {
				if (OS::get_singleton()->is_stdout_verbose()) {

					print_line("has format: "+itos(surf->format));
					print_line("configured format: "+itos(surf->configured_format));
				}
				ERR_EXPLAIN("Missing arrays (not set) in surface");
			}
			ERR_FAIL_COND_V( surf->format != surf->configured_format, ERR_UNCONFIGURED );
			uint8_t *base=0;
			int stride=surf->stride;
			bool use_VBO = (surf->array_local==0);
			_setup_geometry_vinfo=surf->array_len;

			bool skeleton_valid = p_skeleton && (surf->format&VS::ARRAY_FORMAT_BONES) && (surf->format&VS::ARRAY_FORMAT_WEIGHTS) && !p_skeleton->bones.empty() && p_skeleton->bones.size() > surf->max_bone;



			if (!use_VBO) {

				base = surf->array_local;
				// glBindBuffer(GL_ARRAY_BUFFER, 0);
				bool can_copy_to_local=surf->local_stride * surf->array_len <= skinned_buffer_size;
				if (!can_copy_to_local)
					skeleton_valid=false;

				/* compute morphs */

				if (p_morphs && surf->morph_target_count && can_copy_to_local) {

					base = skinned_buffer;
					stride=surf->local_stride;

					//copy all first
					float coef=1.0;

					for(int i=0;i<surf->morph_target_count;i++) {
						if (surf->mesh->morph_target_mode==VS::MORPH_MODE_NORMALIZED)
							coef-=p_morphs[i];
						ERR_FAIL_COND_V( surf->morph_format != surf->morph_targets_local[i].configured_format, ERR_INVALID_DATA );

					}


					for(int i=0;i<VS::ARRAY_MAX-1;i++) {

						const Surface::ArrayData& ad=surf->array[i];
						if (ad.size==0)
							continue;

						int ofs = ad.ofs;
						int src_stride=surf->stride;
						int dst_stride=surf->local_stride;
						int count = surf->array_len;

						switch(i) {

							case VS::ARRAY_VERTEX:
							case VS::ARRAY_NORMAL:
							case VS::ARRAY_TANGENT:
								{

								for(int k=0;k<count;k++) {

									const float *src = (const float*)&surf->array_local[ofs+k*src_stride];
									float *dst = (float*)&base[ofs+k*dst_stride];

									dst[0]= src[0]*coef;
									dst[1]= src[1]*coef;
									dst[2]= src[2]*coef;
								} break;

							} break;
							case VS::ARRAY_COLOR: {

									printf("ARRAY_COLOR\n");
							} break;
							case VS::ARRAY_TEX_UV:
							case VS::ARRAY_TEX_UV2: {

								for(int k=0;k<count;k++) {

									const float *src = (const float*)&surf->array_local[ofs+k*src_stride];
									float *dst = (float*)&base[ofs+k*dst_stride];

									dst[0]= src[0]*coef;
									dst[1]= src[1]*coef;
								} break;

							} break;
							case VS::ARRAY_BONES:
							case VS::ARRAY_WEIGHTS: {
								printf("ARRAY_COLOR2\n");
// 								for (int k = 0; k < count; k++) {
//
// 									const float *src = (const float *)&surf->array_local[ofs + k * src_stride];
// 									float *dst = (float *)&base[ofs + k * dst_stride];
//
// 									dst[0] = src[0];
// 									dst[1] = src[1];
// 									dst[2] = src[2];
// 									dst[3] = src[3];
// 								}

							} break;
						}
					}


					for(int j=0;j<surf->morph_target_count;j++) {

						for(int i=0;i<VS::ARRAY_MAX-1;i++) {

							const Surface::ArrayData& ad=surf->array[i];
							if (ad.size==0)
								continue;


							int ofs = ad.ofs;
							int dst_stride=surf->local_stride;
							int count = surf->array_len;
							const uint8_t *morph=surf->morph_targets_local[j].array;
							float w = p_morphs[j];

							switch(i) {

								case VS::ARRAY_VERTEX:
								case VS::ARRAY_NORMAL:
								case VS::ARRAY_TANGENT:
									{

									for(int k=0;k<count;k++) {

										const float *src_morph = (const float*)&morph[ofs+k*dst_stride];
										float *dst = (float*)&base[ofs+k*dst_stride];

										dst[0]+= src_morph[0]*w;
										dst[1]+= src_morph[1]*w;
										dst[2]+= src_morph[2]*w;
									} break;

								} break;
								case VS::ARRAY_COLOR: {
									printf("ARRAY_COLOR3\n");
// 									for (int k = 0; k < count; k++) {
//
// 										const uint8_t *src = (const uint8_t *)&morph[ofs + k * src_stride];
// 										uint8_t *dst = (uint8_t *)&base[ofs + k * dst_stride];
//
// 										dst[0] = (src[0] * wfp) >> 8;
// 										dst[1] = (src[1] * wfp) >> 8;
// 										dst[2] = (src[2] * wfp) >> 8;
// 										dst[3] = (src[3] * wfp) >> 8;
// 									}

								} break;
								case VS::ARRAY_TEX_UV:
								case VS::ARRAY_TEX_UV2: {

									for(int k=0;k<count;k++) {

										const float *src_morph = (const float*)&morph[ofs+k*dst_stride];
										float *dst = (float*)&base[ofs+k*dst_stride];

										dst[0]+= src_morph[0]*w;
										dst[1]+= src_morph[1]*w;
									} break;

								} break;
							}
						}
					}

				} else if (skeleton_valid) {

					base = skinned_buffer;
					//copy stuff and get it ready for the skeleton

					int len = surf->array_len;
					int src_stride = surf->stride;
					int dst_stride = surf->stride - ( surf->array[VS::ARRAY_BONES].size + surf->array[VS::ARRAY_WEIGHTS].size );

					for(int i=0;i<len;i++) {
						const uint8_t *src = &surf->array_local[i*src_stride];
						uint8_t *dst = &base[i*dst_stride];
						memcpy(dst,src,dst_stride);
					}


					stride=dst_stride;
				}

// 			printf("skeleton_valid%d\n", skeleton_valid);
			if (skeleton_valid) {
					//transform stuff
						const uint8_t *src_weights = &surf->array_local[surf->array[VS::ARRAY_WEIGHTS].ofs];
						const uint8_t *src_bones = &surf->array_local[surf->array[VS::ARRAY_BONES].ofs];
						const Skeleton::Bone *skeleton = &p_skeleton->bones[0];

						if (surf->format & VS::ARRAY_FORMAT_NORMAL && surf->format & VS::ARRAY_FORMAT_TANGENT)
							_skeleton_xform<true, true, true>(base, surf->stride, base, surf->stride, surf->array_len, src_bones, src_weights, skeleton);
						else if (surf->format & (VS::ARRAY_FORMAT_NORMAL))
							_skeleton_xform<true, false, true>(base, surf->stride, base, surf->stride, surf->array_len, src_bones, src_weights, skeleton);
						else if (surf->format & (VS::ARRAY_FORMAT_TANGENT))
							_skeleton_xform<false, true, true>(base, surf->stride, base, surf->stride, surf->array_len, src_bones, src_weights, skeleton);
						else
							_skeleton_xform<false, false, true>(base, surf->stride, base, surf->stride, surf->array_len, src_bones, src_weights, skeleton);


			}

			} else {

				// glBindBuffer(GL_ARRAY_BUFFER, surf->vertex_id);
			};


			for (int i=0;i<(VS::ARRAY_MAX-1);i++) {

				const Surface::ArrayData& ad=surf->array[i];

//				if (!gl_texcoord_shader[i])
//					continue;

				if (ad.size==0 || i==VS::ARRAY_BONES || i==VS::ARRAY_WEIGHTS || gl_client_states[i]==0 ) {

					if (gl_texcoord_index[i] != -1) {
// 						glClientActiveTexture(GL_TEXTURE0+gl_texcoord_index[i]);
					}

					if (gl_client_states[i] != 0)
						// glDisableClientState(gl_client_states[i]);

					if (i == VS::ARRAY_COLOR) {
						// glColor4f(last_color.r,last_color.g,last_color.b,last_color.a);
					};
					continue; // this one is disabled.
				}

				if (gl_texcoord_index[i] != -1) {
// 					glClientActiveTexture(GL_TEXTURE0+gl_texcoord_index[i]);
				}

				// glEnableClientState(gl_client_states[i]);

				switch (i) {

				case VS::ARRAY_VERTEX: {

					// glVertexPointer(3,ad.datatype,stride,&base[ad.ofs]);

				} break; /* fallthrough to normal */
				case VS::ARRAY_NORMAL: {

					// glNormalPointer(ad.datatype,stride,&base[ad.ofs]);
				} break;
				case VS::ARRAY_COLOR: {
					// glColorPointer(4,ad.datatype,stride,&base[ad.ofs]);
				} break;
				case VS::ARRAY_TEX_UV:
				case VS::ARRAY_TEX_UV2: {

					// glTexCoordPointer(2,ad.datatype,stride,&base[ad.ofs]);
				} break;
				case VS::ARRAY_TANGENT: {

					//glVertexAttribPointer(i, 4, use_VBO?GL_BYTE:GL_FLOAT, use_VBO?GL_TRUE:GL_FALSE, stride, &base[ad.ofs]);

				} break;
				case VS::ARRAY_BONES:
				case VS::ARRAY_WEIGHTS: {

					//do none
					//glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, surf->stride, &base[ad.ofs]);

				} break;
				case VS::ARRAY_INDEX:
					ERR_PRINT("Bug");
					break;
				};
			}


		} break;

		default: break;

	};

	return OK;
};

static const GLenum gl_primitive[]={
	GX2_PRIMITIVE_MODE_POINTS,
	GX2_PRIMITIVE_MODE_LINES,
	GX2_PRIMITIVE_MODE_LINE_STRIP,
	GX2_PRIMITIVE_MODE_LINE_LOOP,
	GX2_PRIMITIVE_MODE_TRIANGLES,
	GX2_PRIMITIVE_MODE_TRIANGLE_STRIP,
	GX2_PRIMITIVE_MODE_TRIANGLE_FAN
};

static const GLenum gl_poly_primitive[4]={
	GX2_PRIMITIVE_MODE_POINTS,
	GX2_PRIMITIVE_MODE_LINES,
	GX2_PRIMITIVE_MODE_TRIANGLES,
	//GL_QUADS

};


void RasterizerWIIU::_render(const Geometry *p_geometry,const Material *p_material, const Skeleton* p_skeleton, const GeometryOwner *p_owner) {


	_rinfo.object_count++;

	switch(p_geometry->type) {

		case Geometry::GEOMETRY_SURFACE: {

			Surface *s = (Surface*)p_geometry;

			_rinfo.vertex_count+=s->array_len;

			if (s->packed && s->array_local==0) {

				float sc = (1.0/32767.0)*s->vertex_scale;

				// glMatrixMode(GL_MODELVIEW);
				// glPushMatrix();
				// glScalef(sc,sc,sc);
				if (s->format&VS::ARRAY_FORMAT_TEX_UV) {
					float uvs=(1.0/32767.0)*s->uv_scale;
					////glActiveTexture(GL_TEXTURE0);
// 					glClientActiveTexture(GL_TEXTURE0);
					// glMatrixMode(GL_TEXTURE);
					// glPushMatrix();
					// glScalef(uvs,uvs,uvs);
				}


			}


			if (s->index_array_len>0) {

				if (s->index_array_local) {

					// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
					// glDrawElements(gl_primitive[s->primitive], s->index_array_len, (s->array_len>(1<<16))?GL_UNSIGNED_SHORT:GL_UNSIGNED_SHORT, s->index_array_local);

				} else {
				//	print_line("indices: "+itos(s->index_array_local) );

					// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s->index_id);
					// glDrawElements(gl_primitive[s->primitive],s->index_array_len, (s->array_len>(1<<16))?GL_UNSIGNED_SHORT:GL_UNSIGNED_SHORT,0);
				}


			} else {

				// glDrawArrays(gl_primitive[s->primitive],0,s->array_len);

			};

			if (s->packed && s->array_local==0) {
				if (s->format&VS::ARRAY_FORMAT_TEX_UV) {
					// glPopMatrix();
					// glMatrixMode(GL_MODELVIEW);
				}
				// glPopMatrix();
			};
		} break;

		case Geometry::GEOMETRY_MULTISURFACE: {

			Surface *s = static_cast<const MultiMeshSurface*>(p_geometry)->surface;
			const MultiMesh *mm = static_cast<const MultiMesh*>(p_owner);
			int element_count=mm->elements.size();

			if (element_count==0)
				return;

			const MultiMesh::Element *elements=&mm->elements[0];

			_rinfo.vertex_count+=s->array_len*element_count;


			if (s->index_array_len>0) {


				// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s->index_id);
				for(int i=0;i<element_count;i++) {
					//glUniformMatrix4fv(material_shader.get_uniform_location(MaterialShaderGLES1::INSTANCE_TRANSFORM), 1, false, elements[i].matrix);
					// glDrawElements(gl_primitive[s->primitive],s->index_array_len, (s->array_len>(1<<16))?GL_UNSIGNED_SHORT:GL_UNSIGNED_SHORT,0);
					// GX2DrawIndexedEx(gl_primitive[s->primitive], s->index_array_len, GX2_INDEX_TYPE_U32,
				}


			} else {

				for(int i=0;i<element_count;i++) {
//					glUniformMatrix4fv(material_shader.get_uniform_location(MaterialShaderGLES1::INSTANCE_TRANSFORM), 1, false, elements[i].matrix);
					// GX2DrawEx(gl_primitive[s->primitive],s->array_len, 0, 1);
				}


			 };
		 } break;
		case Geometry::GEOMETRY_PARTICLES: {


			//print_line("particulinas");
			const Particles *particles = static_cast<const Particles*>( p_geometry );
			ERR_FAIL_COND(!p_owner);
			ParticlesInstance *particles_instance = (ParticlesInstance*)p_owner;

			ParticleSystemProcessSW &pp = particles_instance->particles_process;
			float td = time_delta; //MIN(time_delta,1.0/10.0);
			pp.process(&particles->data,particles_instance->transform,td);
			ERR_EXPLAIN("A parameter in the particle system is not correct.");
			ERR_FAIL_COND(!pp.valid);
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0); //unbind
			// glBindBuffer(GL_ARRAY_BUFFER,0);


			Transform camera;
			if (shadow)
				camera=shadow->transform;
			else
				camera=camera_transform;

			particle_draw_info.prepare(&particles->data,&pp,particles_instance->transform,camera);

			_rinfo.vertex_count+=4*particles->data.amount;

			{
				static const Vector3 points[4]={
					Vector3(-1.0,1.0,0),
					Vector3(1.0,1.0,0),
					Vector3(1.0,-1.0,0),
					Vector3(-1.0,-1.0,0)
				};
				static const Vector3 uvs[4]={
					Vector3(0.0,0.0,0.0),
					Vector3(1.0,0.0,0.0),
					Vector3(1.0,1.0,0.0),
					Vector3(0,1.0,0.0)
				};
				static const Vector3 normals[4]={
					Vector3(0,0,1),
					Vector3(0,0,1),
					Vector3(0,0,1),
					Vector3(0,0,1)
				};

				static const Plane tangents[4]={
					Plane(Vector3(1,0,0),0),
					Plane(Vector3(1,0,0),0),
					Plane(Vector3(1,0,0),0),
					Plane(Vector3(1,0,0),0)
				};


				// glMatrixMode(GL_MODELVIEW);
				// glPushMatrix();
				_gl_load_transform(camera_transform_inverse);
				for(int i=0;i<particles->data.amount;i++) {

					ParticleSystemDrawInfoSW::ParticleDrawInfo &pinfo=*particle_draw_info.draw_info_order[i];
					if (!pinfo.data->active)
						continue;
					// glPushMatrix();
					_gl_mult_transform(pinfo.transform);

					// glColor4f(pinfo.color.r*last_color.r,pinfo.color.g*last_color.g,pinfo.color.b*last_color.b,pinfo.color.a*last_color.a);
					// _draw_primitive(4,points,normals,NULL,uvs,tangents); //TODO: WiiU
					// glPopMatrix();

				}
				// glPopMatrix();

			}

		} break;
		 default: break;
	};

};

void RasterizerWIIU::_setup_shader_params(const Material *p_material) {
#if 0
	int idx=0;
	int tex_idx=0;

	for(Map<StringName,Variant>::Element *E=p_material->shader_cache->params.front();E;E=E->next(),idx++) {

		Variant v; //
		v = E->get();
		const Map<StringName,Variant>::Element *F=p_material->shader_params.find(E->key());
		if (F)
			v=F->get();

		switch(v.get_type() ) {
			case Variant::OBJECT:
			case Variant::_RID: {

				RID tex=v;
				if (!tex.is_valid())
					break;

				Texture *texture = texture_owner.get(tex);
				if (!texture)
					break;
				glUniform1i( material_shader.get_custom_uniform_location(idx), tex_idx);
				//glActiveTexture(tex_idx);
				glBindTexture(texture->target,texture->tex_id);

			} break;
			case Variant::COLOR: {

				Color c=v;
				material_shader.set_custom_uniform(idx,Vector3(c.r,c.g,c.b));
			} break;
			default: {

				material_shader.set_custom_uniform(idx,v);
			} break;
		}

	}
#endif

}

void RasterizerWIIU::_render_list_forward(RenderList *p_render_list,bool p_reverse_cull) {

	const Material *prev_material=NULL;
	uint64_t prev_light_key=0;
	const Skeleton *prev_skeleton=NULL;
	const Geometry *prev_geometry=NULL;

	Geometry::Type prev_geometry_type=Geometry::GEOMETRY_INVALID;

	for (int i=0;i<p_render_list->element_count;i++) {

		RenderList::Element *e = p_render_list->elements[i];
		const Material *material = e->material;
		uint64_t light_key = e->light_key;
		const Skeleton *skeleton = e->skeleton;
		const Geometry *geometry = e->geometry;

		if (material!=prev_material || geometry->type!=prev_geometry_type) {
			_setup_material(e->geometry,material);
			_rinfo.mat_change_count++;
			//_setup_material_overrides(e->material,NULL,material_overrides);
			//_setup_material_skeleton(material,skeleton);
		} else {

			if (prev_skeleton!=skeleton) {
				//_setup_material_skeleton(material,skeleton);
			};
		}


		if (geometry!=prev_geometry || geometry->type!=prev_geometry_type  || prev_skeleton!=skeleton) {

			_setup_geometry(geometry, material,e->skeleton,e->instance->morph_values.ptr());
		};

		if (i==0 || light_key!=prev_light_key)
			_setup_lights(e->lights,e->light_count);

		_set_cull(e->mirror,p_reverse_cull);

		// glMatrixMode(GL_MODELVIEW);
		// glPopMatrix();
		// glPushMatrix();


		if (e->instance->billboard || e->instance->depth_scale) {

			Transform xf=e->instance->transform;
			if (e->instance->depth_scale) {

				if (camera_projection.matrix[3][3]) {
					//orthogonal matrix, try to do about the same
					//with viewport size
					//real_t w = Math::abs( 1.0/(2.0*(p_projection.matrix[0][0])) );
					real_t h = Math::abs( 1.0/(2.0*camera_projection.matrix[1][1]) );
					float sc = (h*2.0); //consistent with Y-fov
					xf.basis.scale( Vector3(sc,sc,sc));
				} else {
					//just scale by depth
					real_t sc = -camera_plane.distance_to(xf.origin);
					xf.basis.scale( Vector3(sc,sc,sc));
				}
			}

			if (e->instance->billboard) {

				Vector3 scale = xf.basis.get_scale();
				xf.set_look_at(xf.origin,xf.origin+camera_transform.get_basis().get_axis(2),camera_transform.get_basis().get_axis(1));
				xf.basis.scale(scale);
			}
			_gl_mult_transform(xf); // for fixed pipeline

		} else {
			_gl_mult_transform(e->instance->transform); // for fixed pipeline
		}



		//bool changed_shader = material_shader.bind();
		//if ( changed_shader && material->shader_cache && !material->shader_cache->params.empty())
		//	_setup_shader_params(material);

		_render(geometry, material, skeleton,e->owner);



		prev_material=material;
		prev_skeleton=skeleton;
		prev_geometry=geometry;
		prev_light_key=e->light_key;
		prev_geometry_type=geometry->type;
	}



};



void RasterizerWIIU::end_scene() {

	// glEnable(GL_BLEND);
	// glDepthMask(GL_TRUE);
	// glEnable(GL_DEPTH_TEST);
	// glDisable(GL_SCISSOR_TEST);
	depth_write=true;
	depth_test=true;

	if (scene_fx && scene_fx->skybox_active) {

		//skybox
	} else if (scene_fx && scene_fx->bgcolor_active) {

		// glClearColor(scene_fx->bgcolor.r,scene_fx->bgcolor.g,scene_fx->bgcolor.b,1.0);
		// GX2ClearColor(WHBGfxGetTVColourBuffer(),scene_fx->bgcolor.r,scene_fx->bgcolor.g,scene_fx->bgcolor.b,1.0);
		WHBGfxClearColor(scene_fx->bgcolor.r,scene_fx->bgcolor.g,scene_fx->bgcolor.b,1.0);
	} else {

		// glClearColor(0.3,0.3,0.3,1.0);
		// GX2ClearColor(WHBGfxGetTVColourBuffer(), 0.3,0.3,0.3, 1);
	}
#ifdef GLES_OVER_GL
	//glClearDepth(1.0);
#else
	//glClearDepthf(1.0);
#endif

	// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if (scene_fx && scene_fx->fog_active) {

		/*
		glEnable(GL_FOG);
		glFogf(GL_FOG_MODE,GL_LINEAR);
		glFogf(GL_FOG_DENSITY,scene_fx->fog_attenuation);
		glFogf(GL_FOG_START,scene_fx->fog_near);
		glFogf(GL_FOG_END,scene_fx->fog_far);
		glFogfv(GL_FOG_COLOR,scene_fx->fog_color_far.components);
		glLightfv(GL_LIGHT5,GL_DIFFUSE,scene_fx->fog_color_near.components);

		material_shader.set_conditional( MaterialShaderGLES1::USE_FOG,true);
		*/
	}



	for(int i=0;i<directional_light_count;i++) {

		// glEnable(GL_LIGHT0+i);
		_setup_light(directional_lights[i],i);
	}

	opaque_render_list.sort_mat_light();

	//material_shader.set_uniform_camera(MaterialShaderGLES1::PROJECTION_MATRIX, camera_projection);

	/*
	printf("setting projection to ");
	for (int i=0; i<16; i++) {
		printf("%f, ", ((float*)camera_projection.matrix)[i]);
	};
	printf("\n");

	print_line(String("setting camera to ")+camera_transform_inverse);
	*/
//	material_shader.set_uniform_default(MaterialShaderGLES1::CAMERA_INVERSE, camera_transform_inverse);

	//projection
	//glEnable(GL_RESCALE_NORMAL);

	if (current_env) {

		switch (current_env->bg_mode) {

			case VS::ENV_BG_CANVAS:
			case VS::ENV_BG_KEEP: {
				//copy from framebuffer if framebuffer
				// glClear(GL_DEPTH_BUFFER_BIT);
			} break;
			case VS::ENV_BG_DEFAULT_COLOR:
			case VS::ENV_BG_COLOR: {

				Color bgcolor;
				if (current_env->bg_mode == VS::ENV_BG_COLOR)
					bgcolor = current_env->bg_param[VS::ENV_BG_PARAM_COLOR];
				else
					bgcolor = Globals::get_singleton()->get("render/default_clear_color");
				float a = 1.0;
				// GX2ClearColor(WHBGfxGetTVColourBuffer(), bgcolor.r, bgcolor.g, bgcolor.b, a);
				WHBGfxClearColor(bgcolor.r, bgcolor.g, bgcolor.b, a);
				// glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, a);
				// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			} break;
			case VS::ENV_BG_TEXTURE:
			case VS::ENV_BG_CUBEMAP: {

				// glClear(GL_DEPTH_BUFFER_BIT);
				draw_tex_background = true;
			} break;
		}
	} else {

		Color c = Color(0.3, 0.3, 0.3);
		// glClearColor(c.r, c.g, c.b, 0.0);
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// GX2ClearColor(WHBGfxGetTVColourBuffer(), c.r, c.g, c.b, 0);
	}

	// glEnable(GL_NORMALIZE);

	if(draw_tex_background) {
		_draw_tex_bg();
	}


	// glMatrixMode(GL_PROJECTION);
	// glLoadMatrixf(&camera_projection.matrix[0][0]);
	//modelview (fixedpipie)
	// glMatrixMode(GL_MODELVIEW);
	_gl_load_transform(camera_transform_inverse);
	// glPushMatrix();

	// glDisable(GL_BLEND);

	blend_mode=VS::MATERIAL_BLEND_MODE_MIX;
	lighting=true;
	// glEnable(GL_LIGHTING);
	// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	_render_list_forward(&opaque_render_list);

	current_env = NULL;


	alpha_render_list.sort_z();
	// glEnable(GL_BLEND);

	_render_list_forward(&alpha_render_list);

	// glPopMatrix();


//	material_shader.set_conditional( MaterialShaderGLES1::USE_FOG,false);

	_debug_shadows();
}
void RasterizerWIIU::end_shadow_map() {
#if 0
	ERR_FAIL_COND(!shadow);
	ERR_FAIL_INDEX(shadow_pass,shadow->shadow_buffers.size());

	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);


	ShadowBuffer *sb = shadow->shadow_buffers[shadow_pass];

	ERR_FAIL_COND(!sb);

	glBindFramebuffer(GL_FRAMEBUFFER, sb->fbo);
	glViewport(0, 0, sb->size, sb->size);

	glColorMask(0, 0, 0, 0);

	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(4,8);
	glPolygonOffset( 4.0f, 4096.0f);
	glPolygonOffset( 8.0f, 16.0f);

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	CameraMatrix cm;
	float z_near,z_far;
	Transform light_transform;

	float dp_direction=0.0;
	bool flip_facing=false;

	switch(shadow->base->type) {

		case VS::LIGHT_DIRECTIONAL: {

			cm = shadow->custom_projection;
			light_transform=shadow->custom_transform;
			z_near=cm.get_z_near();
			z_far=cm.get_z_far();

		} break;
		case VS::LIGHT_OMNI: {

			material_shader.set_conditional(MaterialShaderGLES1::USE_DUAL_PARABOLOID,true);
			dp_direction = shadow_pass?1.0:0.0;
			flip_facing = (shadow_pass == 1);
			light_transform=shadow->transform;
			z_near=0;
			z_far=shadow->base->vars[ VS::LIGHT_VAR_RADIUS ];
		} break;
		case VS::LIGHT_SPOT: {

			float far = shadow->base->vars[ VS::LIGHT_VAR_RADIUS ];
			ERR_FAIL_COND( far<=0 );
			float near= far/200.0;
			if (near<0.05)
			 near=0.05;

			float angle = shadow->base->vars[ VS::LIGHT_VAR_SPOT_ANGLE ];

			cm.set_perspective( angle*2.0, 1.0, near, far );
			shadow->projection=cm; // cache
			light_transform=shadow->transform;
			z_near=cm.get_z_near();
			z_far=cm.get_z_far();

		} break;
	}

	Transform light_transform_inverse = light_transform.inverse();

	opaque_render_list.sort_mat();

	glLightf(GL_LIGHT5,GL_LINEAR_ATTENUATION,z_near);
	glLightf(GL_LIGHT5,GL_QUADRATIC_ATTENUATION,z_far);
	glLightf(GL_LIGHT5,GL_CONSTANT_ATTENUATION,dp_direction);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&cm.matrix[0][0]);
	glMatrixMode(GL_MODELVIEW);
	_gl_load_transform(light_transform_inverse);
	glPushMatrix();

	for(int i=0;i<4;i++) {
		for(int j=0;j<3;j++) {

			material_shader.set_conditional(_gl_light_version[i][j],false); //start false by default
		}
		material_shader.set_conditional(_gl_light_shadow[i],false);
	}

	_render_list_forward(&opaque_render_list,flip_facing);

	material_shader.set_conditional(MaterialShaderGLES1::USE_DUAL_PARABOLOID,false);
	glViewport( viewport.x, window_size.height-(viewport.height+viewport.y), viewport.width,viewport.height );
	if (framebuffer.active)
		glBindFramebufferEXT(GL_FRAMEBUFFER,framebuffer.fbo);
	else
		glBindFramebufferEXT(GL_FRAMEBUFFER,0);

	glDisable(GL_POLYGON_OFFSET_FILL);

	glColorMask(1, 1, 1, 1);
	shadow=NULL;
#endif


	ERR_FAIL_COND(!shadow);

	// glDisable(GL_BLEND);
	// glDisable(GL_SCISSOR_TEST);
	// glDisable(GL_DITHER);
	// glEnable(GL_DEPTH_TEST);
	// glDepthMask(true);

	ShadowBuffer *sb = shadow->near_shadow_buffer;

	ERR_FAIL_COND(!sb);

// 	glBindFramebuffer(GL_FRAMEBUFFER, sb->fbo);

// 	if (!use_rgba_shadowmaps)
// 	glColorMask(0, 0, 0, 0);

	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset( 8.0f, 16.0f);

	CameraMatrix cm;
	float z_near, z_far;
	Transform light_transform;

	float dp_direction = 0.0;
	bool flip_facing = false;
	Rect2 vp_rect;

	switch (shadow->base->type) {

		case VS::LIGHT_DIRECTIONAL: {

			if (shadow->base->directional_shadow_mode == VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS) {

				cm = shadow->custom_projection[shadow_pass];
				light_transform = shadow->custom_transform[shadow_pass];

				if (shadow_pass == 0) {

					vp_rect = Rect2(0, sb->size / 2, sb->size / 2, sb->size / 2);
					// glViewport(0, sb->size / 2, sb->size / 2, sb->size / 2);
					// glScissor(0, sb->size / 2, sb->size / 2, sb->size / 2);
				} else if (shadow_pass == 1) {

					vp_rect = Rect2(0, 0, sb->size / 2, sb->size / 2);
					// glViewport(0, 0, sb->size / 2, sb->size / 2);
					// glScissor(0, 0, sb->size / 2, sb->size / 2);

				} else if (shadow_pass == 2) {

					vp_rect = Rect2(sb->size / 2, sb->size / 2, sb->size / 2, sb->size / 2);
					// glViewport(sb->size / 2, sb->size / 2, sb->size / 2, sb->size / 2);
					// glScissor(sb->size / 2, sb->size / 2, sb->size / 2, sb->size / 2);
				} else if (shadow_pass == 3) {

					vp_rect = Rect2(sb->size / 2, 0, sb->size / 2, sb->size / 2);
					// glViewport(sb->size / 2, 0, sb->size / 2, sb->size / 2);
					// glScissor(sb->size / 2, 0, sb->size / 2, sb->size / 2);
				}

				// glEnable(GL_SCISSOR_TEST);

			} else if (shadow->base->directional_shadow_mode == VS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS) {

				if (shadow_pass == 0) {

					cm = shadow->custom_projection[0];
					light_transform = shadow->custom_transform[0];
					vp_rect = Rect2(0, sb->size / 2, sb->size, sb->size / 2);
					// glViewport(0, sb->size / 2, sb->size, sb->size / 2);
					// glScissor(0, sb->size / 2, sb->size, sb->size / 2);
				} else {

					cm = shadow->custom_projection[1];
					light_transform = shadow->custom_transform[1];
					vp_rect = Rect2(0, 0, sb->size, sb->size / 2);
					// glViewport(0, 0, sb->size, sb->size / 2);
					// glScissor(0, 0, sb->size, sb->size / 2);
				}

				// glEnable(GL_SCISSOR_TEST);

			} else {
				cm = shadow->custom_projection[0];
				light_transform = shadow->custom_transform[0];
				vp_rect = Rect2(0, 0, sb->size, sb->size);
				// glViewport(0, 0, sb->size, sb->size);
			}

			z_near = cm.get_z_near();
			z_far = cm.get_z_far();

// 			_glClearDepth(1.0f);
			// glClearColor(1, 1, 1, 1);

// 			if (use_rgba_shadowmaps)
 				// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
// 			else
// 				glClear(GL_DEPTH_BUFFER_BIT);

			// glDisable(GL_SCISSOR_TEST);

		} break;
		case VS::LIGHT_OMNI: {

// 			material_shader.set_conditional(MaterialShaderGLES2::USE_DUAL_PARABOLOID, true);
			dp_direction = shadow_pass ? 1.0 : -1.0;
			flip_facing = (shadow_pass == 1);
			light_transform = shadow->transform;
			z_near = 0;
			z_far = shadow->base->vars[VS::LIGHT_PARAM_RADIUS];
			shadow->dp.x = 1.0 / z_far;
			shadow->dp.y = dp_direction;

			if (shadow_pass == 0) {
				vp_rect = Rect2(0, sb->size / 2, sb->size, sb->size / 2);
				// glViewport(0, sb->size / 2, sb->size, sb->size / 2);
				// glScissor(0, sb->size / 2, sb->size, sb->size / 2);
			} else {
				vp_rect = Rect2(0, 0, sb->size, sb->size / 2);
				// glViewport(0, 0, sb->size, sb->size / 2);
				// glScissor(0, 0, sb->size, sb->size / 2);
			}
			// glEnable(GL_SCISSOR_TEST);
			shadow->projection = cm;

			// glClearColor(1, 1, 1, 1);
// 			_glClearDepth(1.0f);
// 			if (use_rgba_shadowmaps)
 				// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
// 			else
// 				glClear(GL_DEPTH_BUFFER_BIT);
			// glDisable(GL_SCISSOR_TEST);

		} break;
		case VS::LIGHT_SPOT: {

			float far = shadow->base->vars[VS::LIGHT_PARAM_RADIUS];
			ERR_FAIL_COND(far <= 0);
			float near = far / 200.0;
			if (near < 0.05)
				near = 0.05;

			float angle = shadow->base->vars[VS::LIGHT_PARAM_SPOT_ANGLE];

			cm.set_perspective(angle * 2.0, 1.0, near, far);

			shadow->projection = cm; // cache
			light_transform = shadow->transform;
			z_near = cm.get_z_near();
			z_far = cm.get_z_far();

			// glViewport(0, 0, sb->size, sb->size);
			vp_rect = Rect2(0, 0, sb->size, sb->size);
// 			_glClearDepth(1.0f);
			// glClearColor(1, 1, 1, 1);
// 			if (use_rgba_shadowmaps)
				// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
// 			else
// 				glClear(GL_DEPTH_BUFFER_BIT);

		} break;
	}

	Transform light_transform_inverse = light_transform.affine_inverse();

// 	opaque_render_list.sort_mat_geom();
// 	_render_list_forward(&opaque_render_list, light_transform, light_transform_inverse, cm, flip_facing, false);

// 	material_shader.set_conditional(MaterialShaderGLES2::USE_DUAL_PARABOLOID, false);

	//if (!use_rgba_shadowmaps)

	if (shadow_filter == SHADOW_FILTER_ESM) {

		Vector2 psize(1.0 / sb->size, 1.0 / sb->size);
		float pscale = 1.0;
		int passes = shadow->base->vars[VS::LIGHT_PARAM_SHADOW_BLUR_PASSES];
		// glDisable(GL_BLEND);
		// glDisable(GL_CULL_FACE);


// 		for (int i = 0; i < VS::ARRAY_MAX; i++) {
// 			glDisableVertexAttribArray(i);
// 		}
		// glBindBuffer(GL_ARRAY_BUFFER, 0);
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		// glDisable(GL_SCISSOR_TEST);

// 		if (!use_rgba_shadowmaps) {
// 			glEnable(GL_DEPTH_TEST);
// 			glDepthFunc(GL_ALWAYS);
// 			glDepthMask(true);
// 		} else {
			// glDisable(GL_DEPTH_TEST);
// 		}

		for (int i = 0; i < passes; i++) {

			Vector2 src_sb_uv[4] = {
				(vp_rect.pos + Vector2(0, vp_rect.size.y)) / sb->size,
				(vp_rect.pos + vp_rect.size) / sb->size,
				(vp_rect.pos + Vector2(vp_rect.size.x, 0)) / sb->size,
				(vp_rect.pos) / sb->size,
			};
			/*
			Vector2 src_uv[4]={
				Vector2( 0, 1),
				Vector2( 1, 1),
				Vector2( 1, 0),
				Vector2( 0, 0)
			};
*/
			static const Vector2 dst_pos[4] = {
				Vector2(-1, 1),
				Vector2(1, 1),
				Vector2(1, -1),
				Vector2(-1, -1)
			};

// 			glBindFramebuffer(GL_FRAMEBUFFER, blur_shadow_buffer.fbo);
// 			glActiveTexture(GL_TEXTURE0);
			// glBindTexture(GL_TEXTURE_2D, sb->depth);



			_draw_gui_primitive(4, dst_pos, NULL, src_sb_uv);

			Vector2 src_bb_uv[4] = {
				(vp_rect.pos + Vector2(0, vp_rect.size.y)) / blur_shadow_buffer.size,
				(vp_rect.pos + vp_rect.size) / blur_shadow_buffer.size,
				(vp_rect.pos + Vector2(vp_rect.size.x, 0)) / blur_shadow_buffer.size,
				(vp_rect.pos) / blur_shadow_buffer.size,
			};

// 			glBindFramebuffer(GL_FRAMEBUFFER, sb->fbo);
// 			glActiveTexture(GL_TEXTURE0);
			// glBindTexture(GL_TEXTURE_2D, blur_shadow_buffer.depth);

// 			copy_shader.set_conditional(CopyShaderGLES2::SHADOW_BLUR_V_PASS, false);
// 			copy_shader.set_conditional(CopyShaderGLES2::SHADOW_BLUR_H_PASS, true);
// 			copy_shader.bind();
// 			copy_shader.set_uniform(CopyShaderGLES2::PIXEL_SIZE, psize);
// 			copy_shader.set_uniform(CopyShaderGLES2::PIXEL_SCALE, pscale);
// 			copy_shader.set_uniform(CopyShaderGLES2::BLUR_MAGNITUDE, 1);
// 			glUniform1i(copy_shader.get_uniform_location(CopyShaderGLES2::SOURCE), 0);

			_draw_gui_primitive(4, dst_pos, NULL, src_bb_uv);
		}

		// glDepthFunc(GL_LEQUAL);
// 		copy_shader.set_conditional(CopyShaderGLES2::USE_RGBA_DEPTH, false);
// 		copy_shader.set_conditional(CopyShaderGLES2::USE_HIGHP_SOURCE, false);
// 		copy_shader.set_conditional(CopyShaderGLES2::SHADOW_BLUR_V_PASS, false);
// 		copy_shader.set_conditional(CopyShaderGLES2::SHADOW_BLUR_H_PASS, false);
	}

// 	DEBUG_TEST_ERROR("Drawing Shadow");
	shadow = NULL;
// 	glBindFramebuffer(GL_FRAMEBUFFER, current_rt ? current_rt->fbo : base_framebuffer);
	// glColorMask(1, 1, 1, 1);
	//glDisable(GL_POLYGON_OFFSET_FILL);
}

void RasterizerWIIU::_debug_draw_shadow(ShadowBuffer *p_buffer, const Rect2& p_rect) {

/*

	Transform modelview;
	modelview.translate(-(viewport.width / 2.0f), -(viewport.height / 2.0f), 0.0f);
	modelview.scale( Vector3( 2.0f / viewport.width, -2.0f / viewport.height, 1.0f ) );
	modelview.translate(p_rect.pos.x, p_rect.pos.y, 0);
	material_shader.set_uniform_default(MaterialShaderGLES1::MODELVIEW_TRANSFORM, *e->transform);
	glBindTexture(GL_TEXTURE_2D,p_buffer->depth);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	Vector3 coords[4]= {
		Vector3(p_rect.pos.x, p_rect.pos.y, 0 ),
		Vector3(p_rect.pos.x+p_rect.size.width,
		p_rect.pos.y, 0 ),
		Vector3(p_rect.pos.x+p_rect.size.width,
		p_rect.pos.y+p_rect.size.height, 0 ),
		Vector3(p_rect.pos.x,
		p_rect.pos.y+p_rect.size.height, 0 )
	};

	Vector3 texcoords[4]={
		Vector3( 0.0f,0.0f, 0),
		Vector3( 1.0f,0.0f, 0),
		Vector3( 1.0f, 1.0f, 0),
		Vector3( 0.0f, 1.0f, 0),
	};

	_draw_primitive(4,coords,0,0,texcoords);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
*/
}

void RasterizerWIIU::_debug_draw_shadows_type(Vector<ShadowBuffer>& p_shadows,Point2& ofs) {


//	Size2 debug_size(128,128);
	Size2 debug_size(512,512);

	for (int i=0;i<p_shadows.size();i++) {

		ShadowBuffer *sb=&p_shadows[i];

		if (!sb->owner)
			continue;

		if (sb->owner->base->type==VS::LIGHT_DIRECTIONAL) {

			if (sb->owner->shadow_pass!=scene_pass-1)
				continue;
		} else {

			if (sb->owner->shadow_pass!=frame)
				continue;
		}
		_debug_draw_shadow(sb, Rect2( ofs, debug_size ));
		ofs.x+=debug_size.x;
		if ( (ofs.x+debug_size.x) > viewport.width ) {

			ofs.x=0;
			ofs.y+=debug_size.y;
		}
	}

}


void RasterizerWIIU::_debug_shadows() {

	return;
#if 0
	canvas_begin();
	glUseProgram(0);
	glDisable(GL_BLEND);
	Size2 ofs;

	/*
	for(int i=0;i<16;i++) {
		//glActiveTexture(GL_TEXTURE0+i);
		//glDisable(GL_TEXTURE_2D);
	}
	//glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	*/


	_debug_draw_shadows_type(near_shadow_buffers,ofs);
	_debug_draw_shadows_type(far_shadow_buffers,ofs);
#endif
}

void RasterizerWIIU::end_frame() {

	/*
	if (framebuffer.active) {

		canvas_begin(); //resets stuff and goes back to fixedpipe
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		//copy to main bufferz
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D,framebuffer.color);
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2f(-1,-1);
		glTexCoord2f(0,1);
		glVertex2f(-1,+1);
		glTexCoord2f(1,1);
		glVertex2f(+1,+1);
		glTexCoord2f(1,0);
		glVertex2f(+1,-1);
		glEnd();


	}
	*/

	//print_line("VTX: "+itos(_rinfo.vertex_count)+" OBJ: "+itos(_rinfo.object_count)+" MAT: "+itos(_rinfo.mat_change_count)+" SHD: "+itos(_rinfo.shader_change_count));

	// OS::get_singleton()->swap_buffers();
	WHBGfxFinishRenderTV();
	WHBGfxFinishRender();
}

/* CANVAS API */


void RasterizerWIIU::reset_state() {


	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0); //unbind
	// glBindBuffer(GL_ARRAY_BUFFER,0);

	//glActiveTexture(GL_TEXTURE0);
// 	glClientActiveTexture(GL_TEXTURE0);
	// glMatrixMode(GL_TEXTURE);
	// glLoadIdentity();
	// glMatrixMode(GL_PROJECTION);
	// glLoadIdentity();
	// glMatrixMode(GL_MODELVIEW);
	// glLoadIdentity();
	// glColor4f(1,1,1,1);

	// glDisable(GL_CULL_FACE);
	// glDisable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
//	//glBlendEquation(GL_FUNC_ADD);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	canvas_blend=VS::MATERIAL_BLEND_MODE_MIX;
	// glLineWidth(1.0);
	// glDisable(GL_LIGHTING);

}

_FORCE_INLINE_ static void _set_glcoloro(const Color& p_color,const float p_opac) {

	// glColor4f(p_color.r, p_color.g, p_color.b, p_color.a*p_opac);
}


void RasterizerWIIU::canvas_begin() {
	OSReport("canvas_begin\n");

	reset_state();
	canvas_opacity=1.0;
	// glEnable(GL_BLEND);



	canvas_transform = Transform();
	canvas_transform.translate(-(viewport.width / 2.0f), -(viewport.height / 2.0f), 0.0f);
	float csy = 1.0;
	// if (current_rt && current_rt_vflip)
		// csy = -1.0;

	// canvas_transform.scale(Vector3(2.0f / viewport.width, csy * -2.0f / viewport.height, 1.0f));

	const Transform &tr = canvas_transform;

	alignas(0x100) __uint32_t matrix[16]={ /* build a 16x16 matrix */
		tr.basis.elements[0][0],
		tr.basis.elements[1][0],
		tr.basis.elements[2][0],
		0,
		tr.basis.elements[0][1],
		tr.basis.elements[1][1],
		tr.basis.elements[2][1],
		0,
		tr.basis.elements[0][2],
		tr.basis.elements[1][2],
		tr.basis.elements[2][2],
		0,
		tr.origin.x,
		tr.origin.y,
		tr.origin.z,
		1
	};/*
	int x = 0;
	for(int i1 = 0; i1 < 3; i1++) {
	for(int i = 0; i < 3; i++) {
		OSReport("%f\n", tr.basis.elements[i][x]);
	}
	x++;
	}*/

	// OSReport("%f %f %f\n", tr.origin.x,tr.origin.y, tr.origin.z);

	OSReport("Vertex unfirm\n");
	GX2SetVertexUniformBlock(0, sizeof(matrix), (void*)matrix);
	GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_UNIFORM_BLOCK), matrix, sizeof(matrix));

}

void RasterizerWIIU::canvas_disable_blending() {
	print_line("canvas_disable_blending");

	// glDisable(GL_BLEND);
}

void RasterizerWIIU::canvas_set_opacity(float p_opacity) {
	print_line("canvas_set_opacity");

	canvas_opacity = p_opacity;
}

void RasterizerWIIU::canvas_set_blend_mode(VS::MaterialBlendMode p_mode) {
	print_line("canvas_set_blend_mode");
	switch(p_mode) {

		 case VS::MATERIAL_BLEND_MODE_MIX: {
			// GX2SetBlendControl(GX2_RENDER_TARGET_0,

		 } break;
		 case VS::MATERIAL_BLEND_MODE_ADD: {

			////glBlendEquation(GL_FUNC_ADD);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE);

		 } break;
		 case VS::MATERIAL_BLEND_MODE_SUB: {

			////glBlendEquation(GL_FUNC_SUBTRACT);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		 } break;
		case VS::MATERIAL_BLEND_MODE_MUL: {
			////glBlendEquation(GL_FUNC_ADD);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		} break;
		case VS::MATERIAL_BLEND_MODE_PREMULT_ALPHA: {
			//glBlendEquation(GL_FUNC_ADD);
			// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		} break;

	}
	blend_mode = p_mode;

}


inline uint32_t _swapF32(float v)
{
    return __builtin_bswap32(*(uint32_t*)&v);
}

void RasterizerWIIU::canvas_begin_rect(const Matrix32& p_transform) {
	OSReport("canvas_begin_rect\n");
	// glMatrixMode(GL_MODELVIEW);
	// glLoadIdentity();
	// glScalef(2.0 / viewport.width, -2.0 / viewport.height, 0);
	// glTranslatef((-(viewport.width / 2.0)), (-(viewport.height / 2.0)), 0);
	// _gl_mult_transform(p_transform);

	// glPushMatrix();
	const Matrix32 &tr = p_transform;

	projectionMTX[0] = swapf(tr.elements[0][0]);
	projectionMTX[1] = swapf(tr.elements[0][1]);
	projectionMTX[2] = swapf(0);
	projectionMTX[3] = swapf(0);
	projectionMTX[4] = swapf(tr.elements[1][0]);
	projectionMTX[5] = swapf(tr.elements[1][1]);
	projectionMTX[6] = swapf(0);
	projectionMTX[7] = swapf(0);
	projectionMTX[8] = swapf(0);
	projectionMTX[9] = swapf(0);
	projectionMTX[10] = swapf(1);
	projectionMTX[11] = swapf(0);
	projectionMTX[12] = swapf(tr.elements[2][0]);
	projectionMTX[13] = swapf(tr.elements[2][1]);
	projectionMTX[14] = swapf(0);
	projectionMTX[15] = swapf(1);

	/*
	float matrix[]={
		tr.elements[0][0],
		tr.elements[0][1],
		0,
		0,
		tr.elements[1][0],
		tr.elements[1][1],
		0,
		0,
		0,
		0,
		1,
		0,
		tr.elements[2][0],
		tr.elements[2][1],
		0,
		1
	};

	for (int i = 0; i < 16; ++i) {
		OSReport("%d\n", matrix[i]);
	}
	*/
	// for (int i = 0; i < 3; ++i) {
	// 	for(int x = 0; x < 2; x++) {
	// 		OSReport("%f\n", tr.elements[i][x]);
	// 	}
	// }
	// GX2SetVertexShader(shaderGroup.vertexShader);
	// OSReport("Vertex unfirm 1\n");

	GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_UNIFORM_BLOCK), projectionMTX, sizeof(projectionMTX));

}

void RasterizerWIIU::canvas_set_clip(bool p_clip, const Rect2& p_rect) {
	print_line("canvas_set_clip");
	if (p_clip) {

		// glEnable(GL_SCISSOR_TEST);
	//	glScissor(viewport.x+p_rect.pos.x,viewport.y+ (viewport.height-(p_rect.pos.y+p_rect.size.height)),
		//p_rect.size.width,p_rect.size.height);
		//glScissor(p_rect.pos.x,(viewport.height-(p_rect.pos.y+p_rect.size.height)),p_rect.size.width,p_rect.size.height);
		// glScissor(viewport.x+p_rect.pos.x,viewport.y+ (window_size.y-(p_rect.pos.y+p_rect.size.height)),
		// p_rect.size.width,p_rect.size.height);
	} else {

		// glDisable(GL_SCISSOR_TEST);
	}


}

void RasterizerWIIU::canvas_end_rect() {
	print_line("canvas_end_rect");
	// glPopMatrix();
}

void RasterizerWIIU::canvas_draw_line(const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width) {
	print_line("canvas_draw_line");
	// glDisable(GL_TEXTURE_2D);
	_set_glcoloro( p_color,canvas_opacity );

	Vector3 verts[2]={
		Vector3(p_from.x,p_from.y,0),
		Vector3(p_to.x,p_to.y,0)
	};
	Color colors[2]={
		p_color,
		p_color
	};
	colors[0].a*=canvas_opacity;
	colors[1].a*=canvas_opacity;
	// glLineWidth(p_width);
	// _draw_primitive(2,verts,0,colors,0);

}



static void _draw_textured_quad(const Rect2& p_rect, const Rect2& p_src_region, const Size2& p_tex_size,bool p_flip_h=false,bool p_flip_v=false ) {
	float tex_coords[] = {
		 p_src_region.pos.x/p_tex_size.width,
		p_src_region.pos.y/p_tex_size.height,

		(p_src_region.pos.x+p_src_region.size.width)/p_tex_size.width,
		p_src_region.pos.y/p_tex_size.height,

		(p_src_region.pos.x+p_src_region.size.width)/p_tex_size.width,
		(p_src_region.pos.y+p_src_region.size.height)/p_tex_size.height,

		p_src_region.pos.x/p_tex_size.width,
		(p_src_region.pos.y+p_src_region.size.height)/p_tex_size.height,
	};


	Vector3 texcoords[4]= {
		Vector3( p_src_region.pos.x/p_tex_size.width,
		p_src_region.pos.y/p_tex_size.height, 0),

		Vector3((p_src_region.pos.x+p_src_region.size.width)/p_tex_size.width,
		p_src_region.pos.y/p_tex_size.height, 0),

		Vector3( (p_src_region.pos.x+p_src_region.size.width)/p_tex_size.width,
		(p_src_region.pos.y+p_src_region.size.height)/p_tex_size.height, 0),

		Vector3( p_src_region.pos.x/p_tex_size.width,
		(p_src_region.pos.y+p_src_region.size.height)/p_tex_size.height, 0)
	};


	if (!p_flip_h) {
		SWAP( texcoords[0], texcoords[1] );
		SWAP( texcoords[2], texcoords[3] );
	}
	if (!p_flip_v) {
		SWAP( texcoords[1], texcoords[2] );
		SWAP( texcoords[0], texcoords[3] );
	}

	OSReport("x %f y %f w %f h %f \n", p_rect.pos.x, p_rect.pos.y, p_rect.size.width, p_rect.size.height);
/*
	float coords[] = {
		p_rect.pos.x, p_rect.pos.y,0.0f,
		p_rect.pos.x+p_rect.size.width, p_rect.pos.y,0.0f,
		p_rect.pos.x+p_rect.size.width, p_rect.pos.y+p_rect.size.height,0.0f,
		p_rect.pos.x,p_rect.pos.y+p_rect.size.height,0.0f
	};*/

	float tv[] = {
			   -1.0f, -1.0f,0.0f,
    1.0f, -1.0f,0.0f,
    1.0f,  1.0f,0.0f,
   -1.0f,  1.0f,0.0f
	};
	// float tt[] = {
	// 		      0.0f, 1.0f,
 //   1.0f, 1.0f,
 //   1.0f, 0.0f,
 //   0.0f, 0.0f
	// };

	Vector3 coords[4]= {
		Vector3( p_rect.pos.x, p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width, p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width, p_rect.pos.y+p_rect.size.height, 0 ),
		Vector3( p_rect.pos.x,p_rect.pos.y+p_rect.size.height, 0 )
	};

	// GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, tex_coords, 4 * sizeof(*tex_coords));
	// GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, coords, 4 * sizeof(*coords));
/*
	float tt[8] = {
        (coords[0].x - p_rect.pos.x) / p_rect.size.width, (coords[0].y - p_rect.pos.y) / p_rect.size.height,
        (coords[1].x - p_rect.pos.x) / p_rect.size.width, (coords[1].y - p_rect.pos.y) / p_rect.size.height,
        (coords[2].x - p_rect.pos.x) / p_rect.size.width, (coords[2].y - p_rect.pos.y) / p_rect.size.height,
        (coords[3].x - p_rect.pos.x) / p_rect.size.width, (coords[3].y - p_rect.pos.y) / p_rect.size.height
    };*/

	float tt[8] = {
        (coords[0].x) , (coords[0].y ) ,
        (coords[1].x) , (coords[1].y ) ,
        (coords[2].x), (coords[2].y ) ,
        (coords[3].x) , (coords[3].y)
    };

	float flat_texcoords[8] = {
        texcoords[0].x, texcoords[0].y,
        texcoords[1].x, texcoords[1].y,
        texcoords[2].x, texcoords[2].y,
        texcoords[3].x, texcoords[3].y
    };

    // Print the results
 //    for (int i = 0; i < 8; ++i) {
	// 	OSReport("%f\n", tt[i]);
 //
	// }

	_draw_primitive(4,tt,0,0,flat_texcoords);
}

static void _draw_quad(const Rect2& p_rect) {

	Vector3 coords[4]= {
		Vector3( p_rect.pos.x,p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width,p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width,p_rect.pos.y+p_rect.size.height, 0 ),
		Vector3( p_rect.pos.x,p_rect.pos.y+p_rect.size.height, 0 )
	};

	// _draw_primitive(4,coords,0,0,0);

}


void RasterizerWIIU::canvas_draw_rect(const Rect2& p_rect, int p_flags, const Rect2& p_source,RID p_texture,const Color& p_modulate) {
	// OSReport("canvas_draw_rect\n");
	_set_glcoloro( p_modulate,canvas_opacity );

	print_line(p_rect);


	if ( p_texture.is_valid() ) {

		// glEnable(GL_TEXTURE_2D);
		Texture *texture = texture_owner.get( p_texture );
		ERR_FAIL_COND(!texture);
		//glActiveTexture(GL_TEXTURE0);
		// glBindTexture( GL_TEXTURE_2D,texture->tex_id );



		if (!(p_flags&CANVAS_RECT_REGION)) {

			Rect2 region = Rect2(0,0,texture->width,texture->height);
			_draw_textured_quad(p_rect,region,region.size,p_flags&CANVAS_RECT_FLIP_H,p_flags&CANVAS_RECT_FLIP_V);

		} else {


			_draw_textured_quad(p_rect, p_source, Size2(texture->width,texture->height),p_flags&CANVAS_RECT_FLIP_H,p_flags&CANVAS_RECT_FLIP_V );

		}

		GX2SetPixelTexture(&texture->tex_id, shaderGroup.pixelShader->samplerVars[0].location);
		GX2SetPixelSampler(&sampler, shaderGroup.pixelShader->samplerVars[0].location);


		GX2SetVertexUniformBlock(0, sizeof(projectionMTX), (void*)projectionMTX);
		// GX2SetVertexUniformBlock(1, sizeof(modelview_matrix), (void*)modelview_matrix);
		GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);

	} else {

		// glDisable(GL_TEXTURE_2D);
		_draw_quad( p_rect );

	}
}

template <bool use_normalmap>
void RasterizerWIIU::_canvas_item_render_commands(CanvasItem *p_item, CanvasItem *current_clip, bool &reclip) {

	int cc = p_item->commands.size();
	CanvasItem::Command **commands = p_item->commands.ptr();

// 	Matrix32 xform = p_transform * ci->xform;

	for (int i = 0; i < cc; i++) {
		CanvasItem::Command *c = commands[i];

// 		CanvasItem::Command **commands = &cc->commands[0];





		switch (c->type) {
			case CanvasItem::Command::TYPE_LINE: {

				CanvasItem::CommandLine *line = static_cast<CanvasItem::CommandLine *>(c);
				canvas_draw_line(line->from, line->to, line->color, line->width);
			} break;
			case CanvasItem::Command::TYPE_RECT: {

				CanvasItem::CommandRect *rect = static_cast<CanvasItem::CommandRect *>(c);

				int flags = rect->flags;
//  				if (use_normalmap)
// 					_canvas_normal_set_flip(Vector2((flags & CANVAS_RECT_FLIP_H) ? -1 : 1, (flags & CANVAS_RECT_FLIP_V) ? -1 : 1));
				canvas_draw_rect(rect->rect, flags, rect->source, rect->texture, rect->modulate);

			} break;
			case CanvasItem::Command::TYPE_STYLE: {

				CanvasItem::CommandStyle *style = static_cast<CanvasItem::CommandStyle *>(c);
// 				if (use_normalmap)
// 					_canvas_normal_set_flip(Vector2(1, 1));
				canvas_draw_style_box(style->rect, style->source, style->texture, style->margin, style->draw_center, style->color);

			} break;
			case CanvasItem::Command::TYPE_PRIMITIVE: {

//  				if (use_normalmap)
//  					_canvas_normal_set_flip(Vector2(1, 1));
				CanvasItem::CommandPrimitive *primitive = static_cast<CanvasItem::CommandPrimitive *>(c);
				canvas_draw_primitive(primitive->points, primitive->colors, primitive->uvs, primitive->texture, primitive->width);
			} break;
			case CanvasItem::Command::TYPE_POLYGON: {

// 				if (use_normalmap)
//  					_canvas_normal_set_flip(Vector2(1, 1));
				CanvasItem::CommandPolygon *polygon = static_cast<CanvasItem::CommandPolygon *>(c);
				canvas_draw_polygon(polygon->count, polygon->indices.ptr(), polygon->points.ptr(), polygon->uvs.ptr(), polygon->colors.ptr(), polygon->texture, polygon->colors.size() == 1);

			} break;

			case CanvasItem::Command::TYPE_POLYGON_PTR: {

//  				if (use_normalmap)
//  					_canvas_normal_set_flip(Vector2(1, 1));
				CanvasItem::CommandPolygonPtr *polygon = static_cast<CanvasItem::CommandPolygonPtr *>(c);
				canvas_draw_polygon(polygon->count, polygon->indices, polygon->points, polygon->uvs, polygon->colors, polygon->texture, false);
			} break;
			case CanvasItem::Command::TYPE_CIRCLE: {

				CanvasItem::CommandCircle *circle = static_cast<CanvasItem::CommandCircle *>(c);
				static const int numpoints = 32;
				Vector2 points[numpoints + 1];
				points[numpoints] = circle->pos;
				int indices[numpoints * 3];

				for (int i = 0; i < numpoints; i++) {

					points[i] = circle->pos + Vector2(Math::sin(i * Math_PI * 2.0 / numpoints), Math::cos(i * Math_PI * 2.0 / numpoints)) * circle->radius;
					indices[i * 3 + 0] = i;
					indices[i * 3 + 1] = (i + 1) % numpoints;
					indices[i * 3 + 2] = numpoints;
				}
				canvas_draw_polygon(numpoints * 3, indices, points, NULL, &circle->color, RID(), true);
				//canvas_draw_circle(circle->indices.size(),circle->indices.ptr(),circle->points.ptr(),circle->uvs.ptr(),circle->colors.ptr(),circle->texture,circle->colors.size()==1);
			} break;
			case CanvasItem::Command::TYPE_TRANSFORM: {

				CanvasItem::CommandTransform *transform = static_cast<CanvasItem::CommandTransform *>(c);
				canvas_set_transform(transform->xform);
			} break;
			case CanvasItem::Command::TYPE_BLEND_MODE: {

				CanvasItem::CommandBlendMode *bm = static_cast<CanvasItem::CommandBlendMode *>(c);
				canvas_set_blend_mode(bm->blend_mode);

			} break;
			case CanvasItem::Command::TYPE_CLIP_IGNORE: {

				CanvasItem::CommandClipIgnore *ci = static_cast<CanvasItem::CommandClipIgnore *>(c);
				if (current_clip) {

					if (ci->ignore != reclip) {
						if (ci->ignore) {

							// glDisable(GL_SCISSOR_TEST);
							reclip = true;
						} else {

							// glEnable(GL_SCISSOR_TEST);
							//glScissor(viewport.x+current_clip->final_clip_rect.pos.x,viewport.y+ (viewport.height-(current_clip->final_clip_rect.pos.y+current_clip->final_clip_rect.size.height)),
							//current_clip->final_clip_rect.size.width,current_clip->final_clip_rect.size.height);

							int x;
							int y;
							int w;
							int h;

// 							if (current_rt) {
// 								x = current_clip->final_clip_rect.pos.x;
// 								y = current_clip->final_clip_rect.pos.y;
// 								w = current_clip->final_clip_rect.size.x;
// 								h = current_clip->final_clip_rect.size.y;
// 							} else {
								x = current_clip->final_clip_rect.pos.x;
								y = window_size.height - (current_clip->final_clip_rect.pos.y + current_clip->final_clip_rect.size.y);
								w = current_clip->final_clip_rect.size.x;
								h = current_clip->final_clip_rect.size.y;
// 							}

							// glScissor(x, y, w, h);

							reclip = false;
						}
					}
				}

			} break;
		}


	}
}


void RasterizerWIIU::canvas_render_items(CanvasItem *p_item_list, int p_z, const Color &p_modulate, CanvasLight *p_light) {
	print_line("canvas_render_items");
	CanvasItem *current_clip = NULL;
// 	Shader *shader_cache = NULL;

	bool rebind_shader = true;

// 	canvas_opacity = 1.0;
// 	canvas_use_modulate = p_modulate != Color(1, 1, 1, 1);
// 	canvas_modulate = p_modulate;
// 	canvas_shader.set_conditional(CanvasShaderGLES2::USE_MODULATE, canvas_use_modulate);
// 	canvas_shader.set_conditional(CanvasShaderGLES2::USE_DISTANCE_FIELD, false);

	bool reset_modulate = false;
	bool prev_distance_field = false;

	canvas_begin();

	if (p_item_list->clip) {
			canvas_set_clip(true,p_item_list->get_rect());
	}

	while (p_item_list) {

		CanvasItem *ci = p_item_list;



		canvas_begin_rect(ci->final_transform);
		canvas_set_opacity(ci->final_opacity);
		canvas_set_blend_mode(ci->blend_mode);



		if (ci->vp_render) {
			print_line("vp-render");
		}

		if (current_clip != ci->final_clip_owner) {

			current_clip = ci->final_clip_owner;

			//setup clip
			if (current_clip) {

				// glEnable(GL_SCISSOR_TEST);
				//glScissor(viewport.x+current_clip->final_clip_rect.pos.x,viewport.y+ (viewport.height-(current_clip->final_clip_rect.pos.y+current_clip->final_clip_rect.size.height)),
				//current_clip->final_clip_rect.size.width,current_clip->final_clip_rect.size.height);

				/*				int x = viewport.x+current_clip->final_clip_rect.pos.x;
				int y = window_size.height-(viewport.y+current_clip->final_clip_rect.pos.y+current_clip->final_clip_rect.size.y);
				int w = current_clip->final_clip_rect.size.x;
				int h = current_clip->final_clip_rect.size.y;
*/
				int x;
				int y;
				int w;
				int h;

					x = current_clip->final_clip_rect.pos.x;
					y = window_size.height - (current_clip->final_clip_rect.pos.y + current_clip->final_clip_rect.size.y);
					w = current_clip->final_clip_rect.size.x;
					h = current_clip->final_clip_rect.size.y;


				// glScissor(x, y, w, h);

			} else {

				// glDisable(GL_SCISSOR_TEST);
			}
		}



		//begin rect
		CanvasItem *material_owner = ci->material_owner ? ci->material_owner : ci;
		CanvasItemMaterial *material = material_owner->material;



		bool reclip = false;

		if (ci == p_item_list) {

			switch (ci->blend_mode) {

				case VS::MATERIAL_BLEND_MODE_MIX: {
					//glBlendEquation(GL_FUNC_ADD);
// 					if (current_rt && current_rt_transparent) {
// 						glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
// 					} else {
						// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// 					}

				} break;
				case VS::MATERIAL_BLEND_MODE_ADD: {

					//glBlendEquation(GL_FUNC_ADD);
					// glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				} break;
				case VS::MATERIAL_BLEND_MODE_SUB: {

					//glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
					// glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				} break;
				case VS::MATERIAL_BLEND_MODE_MUL: {
					//glBlendEquation(GL_FUNC_ADD);
					// glBlendFunc(GL_DST_COLOR, GL_ZERO);
				} break;
				case VS::MATERIAL_BLEND_MODE_PREMULT_ALPHA: {
					//glBlendEquation(GL_FUNC_ADD);
					// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				} break;
			}

 			blend_mode = ci->blend_mode;
		}

		canvas_opacity = ci->final_opacity;

		if ((p_modulate.a > 0.001 && (!material || material->shading_mode != VS::CANVAS_ITEM_SHADING_ONLY_LIGHT) && !ci->light_masked))
			_canvas_item_render_commands<false>(ci, current_clip, reclip);

		if (blend_mode == VS::MATERIAL_BLEND_MODE_MIX && p_light) {

			CanvasLight *light = p_light;
			bool light_used = false;
			VS::CanvasLightMode mode = VS::CANVAS_LIGHT_MODE_ADD;

			while (light) {

				if (ci->light_mask & light->item_mask && p_z >= light->z_min && p_z <= light->z_max && ci->global_rect_cache.intersects_transformed(light->xform_cache, light->rect_cache)) {

					//intersects this light

					if (!light_used || mode != light->mode) {

						mode = light->mode;

						switch (mode) {

							case VS::CANVAS_LIGHT_MODE_ADD: {
								//glBlendEquation(GL_FUNC_ADD);
								// glBlendFunc(GL_SRC_ALPHA, GL_ONE);

							} break;
							case VS::CANVAS_LIGHT_MODE_SUB: {
								//glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
								// glBlendFunc(GL_SRC_ALPHA, GL_ONE);
							} break;
							case VS::CANVAS_LIGHT_MODE_MIX:
							case VS::CANVAS_LIGHT_MODE_MASK: {
								//glBlendEquation(GL_FUNC_ADD);
								// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

							} break;
						}
					}


// 					bool has_shadow = light->shadow_buffer.is_valid() && ci->light_mask & light->item_shadow_mask;



// 					bool light_rebind = canvas_shader.bind();

					Texture *t = texture_owner.get(light->texture);
					if (!t) {
						// glBindTexture(GL_TEXTURE_2D, white_tex);
					} else {

						// glBindTexture(t->target, t->tex_id);
					}

					_canvas_item_render_commands<true>(ci, current_clip, reclip); //redraw using light
				}

				light = light->next_ptr;
			}

		}

		if (reclip) {

			// glEnable(GL_SCISSOR_TEST);
			//glScissor(viewport.x+current_clip->final_clip_rect.pos.x,viewport.y+ (viewport.height-(current_clip->final_clip_rect.pos.y+current_clip->final_clip_rect.size.height)),
			//current_clip->final_clip_rect.size.width,current_clip->final_clip_rect.size.height);

			int x;
			int y;
			int w;
			int h;

// 			if (current_rt) {
// 				x = current_clip->final_clip_rect.pos.x;
// 				y = current_clip->final_clip_rect.pos.y;
// 				w = current_clip->final_clip_rect.size.x;
// 				h = current_clip->final_clip_rect.size.y;
// 			} else {
				x = current_clip->final_clip_rect.pos.x;
				y = window_size.height - (current_clip->final_clip_rect.pos.y + current_clip->final_clip_rect.size.y);
				w = current_clip->final_clip_rect.size.x;
				h = current_clip->final_clip_rect.size.y;
// 			}

			// glScissor(x, y, w, h);
		}

		p_item_list = p_item_list->next;
		canvas_end_rect();
	}

	if (current_clip) {
		// glDisable(GL_SCISSOR_TEST);
	}
}

void RasterizerWIIU::canvas_draw_style_box(const Rect2 &p_rect, const Rect2 &p_src_region, RID p_texture, const float *p_margin, bool p_draw_center, const Color &p_modulate) {
	print_line("canvas_draw_style_box");
	_set_glcoloro( p_modulate,canvas_opacity );


	Texture *texture = texture_owner.get( p_texture );
	ERR_FAIL_COND(!texture);

	// glEnable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);
	// glBindTexture( GL_TEXTURE_2D,texture->tex_id );


	/* CORNERS */

	_draw_textured_quad( // top left
		Rect2( p_rect.pos, Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_TOP])),
		Rect2( Point2(), Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_TOP])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // top right
		Rect2( Point2( p_rect.pos.x + p_rect.size.width - p_margin[MARGIN_RIGHT], p_rect.pos.y), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_TOP])),
		Rect2( Point2(texture->width-p_margin[MARGIN_RIGHT],0), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_TOP])),
		Size2( texture->width, texture->height ) );


	_draw_textured_quad( // bottom left
		Rect2( Point2(p_rect.pos.x,p_rect.pos.y + p_rect.size.height - p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_BOTTOM])),
		Rect2( Point2(0,texture->height-p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_BOTTOM])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // bottom right
		Rect2( Point2( p_rect.pos.x + p_rect.size.width - p_margin[MARGIN_RIGHT], p_rect.pos.y + p_rect.size.height - p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_BOTTOM])),
		Rect2( Point2(texture->width-p_margin[MARGIN_RIGHT],texture->height-p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_BOTTOM])),
		Size2( texture->width, texture->height ) );

	Rect2 rect_center( p_rect.pos+Point2( p_margin[MARGIN_LEFT], p_margin[MARGIN_TOP]), Size2( p_rect.size.width - p_margin[MARGIN_LEFT] - p_margin[MARGIN_RIGHT], p_rect.size.height - p_margin[MARGIN_TOP] - p_margin[MARGIN_BOTTOM] ));

	Rect2 src_center( Point2( p_margin[MARGIN_LEFT], p_margin[MARGIN_TOP]), Size2( texture->width - p_margin[MARGIN_LEFT] - p_margin[MARGIN_RIGHT], texture->height - p_margin[MARGIN_TOP] - p_margin[MARGIN_BOTTOM] ));


	_draw_textured_quad( // top
		Rect2( Point2(rect_center.pos.x,p_rect.pos.y),Size2(rect_center.size.width,p_margin[MARGIN_TOP])),
		Rect2( Point2(p_margin[MARGIN_LEFT],0), Size2(src_center.size.width,p_margin[MARGIN_TOP])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // bottom
		Rect2( Point2(rect_center.pos.x,rect_center.pos.y+rect_center.size.height),Size2(rect_center.size.width,p_margin[MARGIN_BOTTOM])),
		Rect2( Point2(p_margin[MARGIN_LEFT],src_center.pos.y+src_center.size.height), Size2(src_center.size.width,p_margin[MARGIN_BOTTOM])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // left
		Rect2( Point2(p_rect.pos.x,rect_center.pos.y),Size2(p_margin[MARGIN_LEFT],rect_center.size.height)),
		Rect2( Point2(0,p_margin[MARGIN_TOP]), Size2(p_margin[MARGIN_LEFT],src_center.size.height)),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // right
		Rect2( Point2(rect_center.pos.x+rect_center.size.width,rect_center.pos.y),Size2(p_margin[MARGIN_RIGHT],rect_center.size.height)),
		Rect2( Point2(src_center.pos.x+src_center.size.width,p_margin[MARGIN_TOP]), Size2(p_margin[MARGIN_RIGHT],src_center.size.height)),
		Size2( texture->width, texture->height ) );

	if (p_draw_center) {

		_draw_textured_quad(
			rect_center,
			src_center,
			Size2( texture->width, texture->height ));
	}

}
void RasterizerWIIU::canvas_draw_primitive(const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture,float p_width) {
	print_line("canvas_draw_primitive");
	ERR_FAIL_COND(p_points.size()<1);
	Vector3 verts[4];
	Vector3 uvs[4];

	_set_glcoloro( Color(1,1,1),canvas_opacity );

	for(int i=0;i<p_points.size();i++) {

		verts[i]=Vector3(p_points[i].x,p_points[i].y,0);
	}

	for(int i=0;i<p_uvs.size();i++) {

		uvs[i]=Vector3(p_uvs[i].x,p_uvs[i].y,0);
	}

	// _draw_primitive(p_points.size(),&verts[0],NULL,p_colors.size()?&p_colors[0]:NULL,p_uvs.size()?uvs:NULL);

	if (p_texture.is_valid()) {
		// glEnable(GL_TEXTURE_2D);
		Texture *texture = texture_owner.get( p_texture );
		if (texture) {
			//glActiveTexture(GL_TEXTURE0);
			// glBindTexture( GL_TEXTURE_2D,texture->tex_id );

			// GX2SetPixelTexture(&texture->tex_id, group.pixelShader->samplerVars[0].location);
			// GX2SetPixelSampler(&sampler, group.pixelShader->samplerVars[0].location);
		}
	}
	// GX2DrawEx(prim_type[p_points.size() - 1], p_points.size(), 0, 1);
	// glLineWidth(p_width);


}

static const int _max_draw_poly_indices = 8*1024;
static uint16_t _draw_poly_indices[_max_draw_poly_indices];
static float _verts3[_max_draw_poly_indices];

void RasterizerWIIU::canvas_draw_polygon(int p_vertex_count, const int* p_indices, const Vector2* p_vertices, const Vector2* p_uvs, const Color* p_colors,const RID& p_texture,bool p_singlecolor) {
	print_line("canvas_draw_polygon");
	bool do_colors=false;

	//reset_state();
	if (p_singlecolor) {
		Color m = *p_colors;
		m.a*=canvas_opacity;
		// glColor4f(m.r, m.g, m.b, m.a);
	} else if (!p_colors) {
		// glColor4f(1, 1, 1, canvas_opacity);
	} else
		do_colors=true;

	// glColor4f(1, 1, 1, 1);

	Texture* texture = NULL;
	if (p_texture.is_valid()) {
		// glEnable(GL_TEXTURE_2D);
		texture = texture_owner.get( p_texture );
		if (texture) {
			//glActiveTexture(GL_TEXTURE0);
			// glBindTexture( GL_TEXTURE_2D,texture->tex_id );
		}
	}

	// glEnableClientState(GL_VERTEX_ARRAY);
	// glVertexPointer(2, GL_FLOAT, 0, (GLvoid*)p_vertices);
	// GX2SetAttribBuffer(0, sizeof(p_vertices)*sizeof(float), ;
	if (do_colors) {

		// glEnableClientState(GL_COLOR_ARRAY);
		// glColorPointer(4,GL_FLOAT, 0, p_colors);

	} else {
		// glDisableClientState(GL_COLOR_ARRAY);
	}

	if (texture && p_uvs) {

// 		glClientActiveTexture(GL_TEXTURE0);
		// glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		// glTexCoordPointer(2, GL_FLOAT, 0, p_uvs);

	} else {
		// glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (p_indices) {

		for (int i=0; i<p_vertex_count; i++) {
			_draw_poly_indices[i] = p_indices[i];
		};
		// glDrawElements(GL_TRIANGLES, p_vertex_count, GL_UNSIGNED_SHORT, _draw_poly_indices );
	} else {

		// glDrawArrays(GL_TRIANGLES,0,p_vertex_count);
		// GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLES, p_vertex_count, 0, 1);
	}

	// glDisableClientState(GL_VERTEX_ARRAY);
	// glDisableClientState(GL_COLOR_ARRAY);
	// glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

RID RasterizerWIIU::canvas_light_occluder_create() {

	CanvasOccluder *co = memnew(CanvasOccluder);
	co->index_id = 0;
	co->vertex_id = 0;
	co->len = 0;

	return canvas_occluder_owner.make_rid(co);
}


void RasterizerWIIU::canvas_light_occluder_set_polylines(RID p_occluder, const DVector<Vector2> &p_lines) {

	CanvasOccluder *co = canvas_occluder_owner.get(p_occluder);
	ERR_FAIL_COND(!co);

	co->lines = p_lines;

	if (p_lines.size() != co->len) {

		// if (co->index_id)
			// glDeleteBuffers(1, &co->index_id);
		// if (co->vertex_id)
			// glDeleteBuffers(1, &co->vertex_id);

		co->index_id = 0;
		co->vertex_id = 0;
		co->len = 0;
	}

	if (p_lines.size()) {

		DVector<float> geometry;
		DVector<uint16_t> indices;
		int lc = p_lines.size();

		geometry.resize(lc * 6);
		indices.resize(lc * 3);

		DVector<float>::Write vw = geometry.write();
		DVector<uint16_t>::Write iw = indices.write();

		DVector<Vector2>::Read lr = p_lines.read();

		const int POLY_HEIGHT = 16384;

		for (int i = 0; i < lc / 2; i++) {

			vw[i * 12 + 0] = lr[i * 2 + 0].x;
			vw[i * 12 + 1] = lr[i * 2 + 0].y;
			vw[i * 12 + 2] = POLY_HEIGHT;

			vw[i * 12 + 3] = lr[i * 2 + 1].x;
			vw[i * 12 + 4] = lr[i * 2 + 1].y;
			vw[i * 12 + 5] = POLY_HEIGHT;

			vw[i * 12 + 6] = lr[i * 2 + 1].x;
			vw[i * 12 + 7] = lr[i * 2 + 1].y;
			vw[i * 12 + 8] = -POLY_HEIGHT;

			vw[i * 12 + 9] = lr[i * 2 + 0].x;
			vw[i * 12 + 10] = lr[i * 2 + 0].y;
			vw[i * 12 + 11] = -POLY_HEIGHT;

			iw[i * 6 + 0] = i * 4 + 0;
			iw[i * 6 + 1] = i * 4 + 1;
			iw[i * 6 + 2] = i * 4 + 2;

			iw[i * 6 + 3] = i * 4 + 2;
			iw[i * 6 + 4] = i * 4 + 3;
			iw[i * 6 + 5] = i * 4 + 0;
		}

		//if same buffer len is being set, just use BufferSubData to avoid a pipeline flush

		if (!co->vertex_id) {
			// glGenBuffers(1, &co->vertex_id);
			// glBindBuffer(GL_ARRAY_BUFFER, co->vertex_id);
			// glBufferData(GL_ARRAY_BUFFER, lc * 6 * sizeof(real_t), vw.ptr(), GL_STATIC_DRAW);
		} else {

			// glBindBuffer(GL_ARRAY_BUFFER, co->vertex_id);
			// glBufferSubData(GL_ARRAY_BUFFER, 0, lc * 6 * sizeof(real_t), vw.ptr());
		}

		// glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind

		if (!co->index_id) {

			// glGenBuffers(1, &co->index_id);
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, co->index_id);
			// glBufferData(GL_ELEMENT_ARRAY_BUFFER, lc * 3 * sizeof(uint16_t), iw.ptr(), GL_STATIC_DRAW);
		} else {

			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, co->index_id);
			// glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, lc * 3 * sizeof(uint16_t), iw.ptr());
		}

		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbind

		co->len = lc;
	}
}

void RasterizerWIIU::canvas_set_transform(const Matrix32& p_transform) {
	OSReport("canvas_set_transform\n");
	//restore
	// glPopMatrix();
	// glPushMatrix();
	//set
	// _gl_mult_transform(p_transform);
	const Matrix32 &tr = p_transform;
/*
	alignas(0x100) __uint32_t matrix[16] = {
		tr.elements[0][0],
		tr.elements[0][1],
		0,
		0,
		tr.elements[1][0],
		tr.elements[1][1],
		0,
		0,
		0,
		0,
		1,
		0,
		tr.elements[2][0],
		tr.elements[2][1],
		0,
		1
	};*/
	projectionMTX[16] = swapf(tr.elements[0][0]);
	projectionMTX[17] = swapf(tr.elements[0][1]);
	projectionMTX[18] = swapf(0);
	projectionMTX[19] = swapf(0);
	projectionMTX[20] = swapf(tr.elements[1][0]);
	projectionMTX[21] = swapf(tr.elements[1][1]);
	projectionMTX[22] = swapf(0);
	projectionMTX[23] = swapf(0);
	projectionMTX[24] = swapf(0);
	projectionMTX[25] = swapf(0);
	projectionMTX[26] = swapf(1);
	projectionMTX[27] = swapf(0);
	projectionMTX[28] = swapf(tr.elements[2][0]);
	projectionMTX[29] = swapf(tr.elements[2][1]);
	projectionMTX[30] = swapf(0);
	projectionMTX[31] = swapf(1);
	// GX2SetVertexShader(shaderGroup.vertexShader);
	// OSReport("Vertex unfirm\n");
	// GX2SetVertexUniformBlock(2, sizeof(matrix), (void*)matrix);
	// GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_UNIFORM_BLOCK), modelview_matrix, sizeof(modelview_matrix));
}

/* FX */


RID RasterizerWIIU::fx_create() {

	FX *fx = memnew( FX );
	ERR_FAIL_COND_V(!fx,RID());
	return fx_owner.make_rid(fx);

}
void RasterizerWIIU::fx_get_effects(RID p_fx,List<String> *p_effects) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);

	p_effects->clear();
	p_effects->push_back("bgcolor");
	p_effects->push_back("skybox");
	p_effects->push_back("antialias");
	//p_effects->push_back("hdr");
	p_effects->push_back("glow");	// glow has a bloom parameter, too
	p_effects->push_back("ssao");
	p_effects->push_back("fog");
	p_effects->push_back("dof_blur");
	p_effects->push_back("toon");
	p_effects->push_back("edge");

}
void RasterizerWIIU::fx_set_active(RID p_fx,const String& p_effect, bool p_active) {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);

	if (p_effect=="bgcolor")
		fx->bgcolor_active=p_active;
	else if (p_effect=="skybox")
		fx->skybox_active=p_active;
	else if (p_effect=="antialias")
		fx->antialias_active=p_active;
	else if (p_effect=="glow")
		fx->glow_active=p_active;
	else if (p_effect=="ssao")
		fx->ssao_active=p_active;
	else if (p_effect=="fog")
		fx->fog_active=p_active;
//	else if (p_effect=="dof_blur")
//		fx->dof_blur_active=p_active;
	else if (p_effect=="toon")
		fx->toon_active=p_active;
	else if (p_effect=="edge")
		fx->edge_active=p_active;
}
bool RasterizerWIIU::fx_is_active(RID p_fx,const String& p_effect) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND_V(!fx,false);

	if (p_effect=="bgcolor")
		return fx->bgcolor_active;
	else if (p_effect=="skybox")
		return fx->skybox_active;
	else if (p_effect=="antialias")
		return fx->antialias_active;
	else if (p_effect=="glow")
		return fx->glow_active;
	else if (p_effect=="ssao")
		return fx->ssao_active;
	else if (p_effect=="fog")
		return fx->fog_active;
	//else if (p_effect=="dof_blur")
	//	return fx->dof_blur_active;
	else if (p_effect=="toon")
		return fx->toon_active;
	else if (p_effect=="edge")
		return fx->edge_active;

	return false;
}
void RasterizerWIIU::fx_get_effect_params(RID p_fx,const String& p_effect,List<PropertyInfo> *p_params) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);


	if (p_effect=="bgcolor") {

		p_params->push_back( PropertyInfo( Variant::COLOR, "color" ) );
	} else if (p_effect=="skybox") {
		p_params->push_back( PropertyInfo( Variant::_RID, "cubemap" ) );
	} else if (p_effect=="antialias") {

		p_params->push_back( PropertyInfo( Variant::REAL, "tolerance", PROPERTY_HINT_RANGE,"1,128,1" ) );

	} else if (p_effect=="glow") {

		p_params->push_back( PropertyInfo( Variant::INT, "passes", PROPERTY_HINT_RANGE,"1,4,1" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "attenuation", PROPERTY_HINT_RANGE,"0.01,8.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "bloom", PROPERTY_HINT_RANGE,"-1.0,1.0,0.01" ) );

	} else if (p_effect=="ssao") {

		p_params->push_back( PropertyInfo( Variant::REAL, "radius", PROPERTY_HINT_RANGE,"0.0,16.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "max_distance", PROPERTY_HINT_RANGE,"0.0,256.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "range_max", PROPERTY_HINT_RANGE,"0.0,1.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "range_min", PROPERTY_HINT_RANGE,"0.0,1.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "attenuation", PROPERTY_HINT_RANGE,"0.0,8.0,0.01" ) );

	} else if (p_effect=="fog") {

		p_params->push_back( PropertyInfo( Variant::REAL, "begin", PROPERTY_HINT_RANGE,"0.0,8192,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "end", PROPERTY_HINT_RANGE,"0.0,8192,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "attenuation", PROPERTY_HINT_RANGE,"0.0,8.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::COLOR, "color_begin" ) );
		p_params->push_back( PropertyInfo( Variant::COLOR, "color_end" ) );
		p_params->push_back( PropertyInfo( Variant::BOOL, "fog_bg" ) );

//	} else if (p_effect=="dof_blur") {
//		return fx->dof_blur_active;
	} else if (p_effect=="toon") {
		p_params->push_back( PropertyInfo( Variant::REAL, "treshold", PROPERTY_HINT_RANGE,"0.0,1.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "soft", PROPERTY_HINT_RANGE,"0.001,1.0,0.001" ) );
	} else if (p_effect=="edge") {

	}
}
Variant RasterizerWIIU::fx_get_effect_param(RID p_fx,const String& p_effect,const String& p_param) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND_V(!fx,Variant());

	if (p_effect=="bgcolor") {

		if (p_param=="color")
			return fx->bgcolor;
	} else if (p_effect=="skybox") {
		if (p_param=="cubemap")
			return fx->skybox_cubemap;
	} else if (p_effect=="antialias") {

		if (p_param=="tolerance")
			return fx->antialias_tolerance;

	} else if (p_effect=="glow") {

		if (p_param=="passes")
			return fx->glow_passes;
		if (p_param=="attenuation")
			return fx->glow_attenuation;
		if (p_param=="bloom")
			return fx->glow_bloom;

	} else if (p_effect=="ssao") {

		if (p_param=="attenuation")
			return fx->ssao_attenuation;
		if (p_param=="max_distance")
			return fx->ssao_max_distance;
		if (p_param=="range_max")
			return fx->ssao_range_max;
		if (p_param=="range_min")
			return fx->ssao_range_min;
		if (p_param=="radius")
			return fx->ssao_radius;

	} else if (p_effect=="fog") {

		if (p_param=="begin")
			return fx->fog_near;
		if (p_param=="end")
			return fx->fog_far;
		if (p_param=="attenuation")
			return fx->fog_attenuation;
		if (p_param=="color_begin")
			return fx->fog_color_near;
		if (p_param=="color_end")
			return fx->fog_color_far;
		if (p_param=="fog_bg")
			return fx->fog_bg;
//	} else if (p_effect=="dof_blur") {
//		return fx->dof_blur_active;
	} else if (p_effect=="toon") {
		if (p_param=="treshold")
			return fx->toon_treshold;
		if (p_param=="soft")
			return fx->toon_soft;

	} else if (p_effect=="edge") {

	}
	return Variant();
}
void RasterizerWIIU::fx_set_effect_param(RID p_fx,const String& p_effect, const String& p_param, const Variant& p_value) {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);

	if (p_effect=="bgcolor") {

		if (p_param=="color")
			fx->bgcolor=p_value;
	} else if (p_effect=="skybox") {
		if (p_param=="cubemap")
			fx->skybox_cubemap=p_value;

	} else if (p_effect=="antialias") {

		if (p_param=="tolerance")
			fx->antialias_tolerance=p_value;

	} else if (p_effect=="glow") {

		if (p_param=="passes")
			fx->glow_passes=p_value;
		if (p_param=="attenuation")
			fx->glow_attenuation=p_value;
		if (p_param=="bloom")
			fx->glow_bloom=p_value;

	} else if (p_effect=="ssao") {

		if (p_param=="attenuation")
			fx->ssao_attenuation=p_value;
		if (p_param=="radius")
			fx->ssao_radius=p_value;
		if (p_param=="max_distance")
			fx->ssao_max_distance=p_value;
		if (p_param=="range_max")
			fx->ssao_range_max=p_value;
		if (p_param=="range_min")
			fx->ssao_range_min=p_value;

	} else if (p_effect=="fog") {

		if (p_param=="begin")
			fx->fog_near=p_value;
		if (p_param=="end")
			fx->fog_far=p_value;
		if (p_param=="attenuation")
			fx->fog_attenuation=p_value;
		if (p_param=="color_begin")
			fx->fog_color_near=p_value;
		if (p_param=="color_end")
			fx->fog_color_far=p_value;
		if (p_param=="fog_bg")
			fx->fog_bg=p_value;
//	} else if (p_effect=="dof_blur") {
//		fx->dof_blur_active=p_value;
	} else if (p_effect=="toon") {

		if (p_param=="treshold")
			fx->toon_treshold=p_value;
		if (p_param=="soft")
			fx->toon_soft=p_value;

	} else if (p_effect=="edge") {

	}

}

/* ENVIRONMENT */

RID RasterizerWIIU::environment_create() {

	Environment * env = memnew( Environment );
	return environment_owner.make_rid(env);
}

void RasterizerWIIU::environment_set_background(RID p_env,VS::EnvironmentBG p_bg) {

	ERR_FAIL_INDEX(p_bg,VS::ENV_BG_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->bg_mode=p_bg;
}

VS::EnvironmentBG RasterizerWIIU::environment_get_background(RID p_env) const{

	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,VS::ENV_BG_MAX);
	return env->bg_mode;
}

void RasterizerWIIU::environment_set_background_param(RID p_env,VS::EnvironmentBGParam p_param, const Variant& p_value){

	ERR_FAIL_INDEX(p_param,VS::ENV_BG_PARAM_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->bg_param[p_param]=p_value;

}
Variant RasterizerWIIU::environment_get_background_param(RID p_env,VS::EnvironmentBGParam p_param) const{

	ERR_FAIL_INDEX_V(p_param,VS::ENV_BG_PARAM_MAX,Variant());
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,Variant());
	return env->bg_param[p_param];

}

void RasterizerWIIU::environment_set_enable_fx(RID p_env,VS::EnvironmentFx p_effect,bool p_enabled){

	ERR_FAIL_INDEX(p_effect,VS::ENV_FX_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->fx_enabled[p_effect]=p_enabled;
}
bool RasterizerWIIU::environment_is_fx_enabled(RID p_env,VS::EnvironmentFx p_effect) const{

	ERR_FAIL_INDEX_V(p_effect,VS::ENV_FX_MAX,false);
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,false);
	return env->fx_enabled[p_effect];

}

void RasterizerWIIU::environment_fx_set_param(RID p_env,VS::EnvironmentFxParam p_param,const Variant& p_value){

	ERR_FAIL_INDEX(p_param,VS::ENV_FX_PARAM_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->fx_param[p_param]=p_value;
}
Variant RasterizerWIIU::environment_fx_get_param(RID p_env,VS::EnvironmentFxParam p_param) const{

	ERR_FAIL_INDEX_V(p_param,VS::ENV_FX_PARAM_MAX,Variant());
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,Variant());
	return env->fx_param[p_param];

}

/* SAMPLED LIGHT */

RID RasterizerWIIU::sampled_light_dp_create(int p_width,int p_height) {
// 	printf("SAMPLED LIGHT \n");
	SampledLight *slight = memnew(SampledLight);
	slight->w = p_width;
	slight->h = p_height;
	slight->multiplier = 1.0;
	slight->is_float = false;

// 	glActiveTexture(GL_TEXTURE0);
	// glGenTextures(1, &slight->texture);
	// glBindTexture(GL_TEXTURE_2D, slight->texture);
	// for debug, but glitchy
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Remove artifact on the edges of the shadowmap
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (slight->is_float) {
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p_width, p_height, 0, GL_RGBA, GL_FLOAT, NULL);
	} else {

		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p_width, p_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	return sampled_light_owner.make_rid(slight);
}

void RasterizerWIIU::sampled_light_dp_update(RID p_sampled_light, const Color *p_data, float p_multiplier) {
// 	printf("SAMPLED LIGHT UPDATE\n");
	SampledLight *slight = sampled_light_owner.get(p_sampled_light);
	ERR_FAIL_COND(!slight);

	// glBindTexture(GL_TEXTURE_2D, slight->texture);

	if (slight->is_float) {

		// glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, slight->w, slight->h, GL_RGBA, GL_FLOAT, p_data);

	} else {
		//convert to bytes
		uint8_t *tex8 = (uint8_t *)alloca(slight->w * slight->h * 4);
		const float *src = (const float *)p_data;

		for (int i = 0; i < slight->w * slight->h * 4; i++) {

			tex8[i] = Math::fast_ftoi(CLAMP(src[i] * 255.0, 0.0, 255.0));
		}

		// glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, slight->w, slight->h, GL_RGBA, GL_UNSIGNED_BYTE, p_data);
	}

	slight->multiplier = p_multiplier;

}

/*MISC*/

bool RasterizerWIIU::is_texture(const RID& p_rid) const {

	return texture_owner.owns(p_rid);
}
bool RasterizerWIIU::is_material(const RID& p_rid) const {

	return material_owner.owns(p_rid);
}
bool RasterizerWIIU::is_mesh(const RID& p_rid) const {

	return mesh_owner.owns(p_rid);
}

bool RasterizerWIIU::is_immediate(const RID& p_rid) const {

	return immediate_owner.owns(p_rid);
}

bool RasterizerWIIU::is_multimesh(const RID& p_rid) const {

	return multimesh_owner.owns(p_rid);
}
bool RasterizerWIIU::is_particles(const RID &p_beam) const {

	return particles_owner.owns(p_beam);
}

bool RasterizerWIIU::is_light(const RID& p_rid) const {

	return light_owner.owns(p_rid);
}
bool RasterizerWIIU::is_light_instance(const RID& p_rid) const {

	return light_instance_owner.owns(p_rid);
}
bool RasterizerWIIU::is_particles_instance(const RID& p_rid) const {

	return particles_instance_owner.owns(p_rid);
}
bool RasterizerWIIU::is_skeleton(const RID& p_rid) const {

	return skeleton_owner.owns(p_rid);
}
bool RasterizerWIIU::is_environment(const RID& p_rid) const {

	return environment_owner.owns(p_rid);
}
bool RasterizerWIIU::is_fx(const RID& p_rid) const {

	return fx_owner.owns(p_rid);
}
bool RasterizerWIIU::is_shader(const RID& p_rid) const {

	return false;
}

void RasterizerWIIU::free(const RID& p_rid) {

	if (texture_owner.owns(p_rid)) {

		// delete the texture
		Texture *texture = texture_owner.get(p_rid);

		// glDeleteTextures( 1,&texture->tex_id );
		_rinfo.texture_mem-=texture->total_data_size;
		texture_owner.free(p_rid);
		memdelete(texture);

	} else if (shader_owner.owns(p_rid)) {

		// delete the texture
		Shader *shader = shader_owner.get(p_rid);



		shader_owner.free(p_rid);
		memdelete(shader);

	} else if (material_owner.owns(p_rid)) {

		Material *material = material_owner.get( p_rid );
		ERR_FAIL_COND(!material);

		material_owner.free(p_rid);
		memdelete(material);

	} else if (mesh_owner.owns(p_rid)) {

		Mesh *mesh = mesh_owner.get(p_rid);
		ERR_FAIL_COND(!mesh);
		for (int i=0;i<mesh->surfaces.size();i++) {

			Surface *surface = mesh->surfaces[i];
			if (surface->array_local != 0) {
				memfree(surface->array_local);
			};
			if (surface->index_array_local != 0) {
				memfree(surface->index_array_local);
			};

			if (mesh->morph_target_count>0) {

				for(int i=0;i<mesh->morph_target_count;i++) {

					memfree(surface->morph_targets_local[i].array);
				}
				memfree(surface->morph_targets_local);
				surface->morph_targets_local=NULL;
			}

			// if (surface->vertex_id)
				// glDeleteBuffers(1,&surface->vertex_id);
			// if (surface->index_id)
				// glDeleteBuffers(1,&surface->index_id);

			memdelete( surface );
		};

		mesh->surfaces.clear();

		mesh_owner.free(p_rid);
		memdelete(mesh);

	} else if (multimesh_owner.owns(p_rid)) {

	       MultiMesh *multimesh = multimesh_owner.get(p_rid);
	       ERR_FAIL_COND(!multimesh);

	       multimesh_owner.free(p_rid);
	       memdelete(multimesh);

	} else if (skeleton_owner.owns(p_rid)) {

		Skeleton *skeleton = skeleton_owner.get(p_rid);
		ERR_FAIL_COND(!skeleton);

		if (skeleton->dirty_list.in_list())
			_skeleton_dirty_list.remove(&skeleton->dirty_list);
		if (skeleton->tex_id) {
			// glDeleteTextures(1, &skeleton->tex_id);
		}
		skeleton_owner.free(p_rid);
		memdelete(skeleton);
	} else if (particles_owner.owns(p_rid)) {

		Particles *particles = particles_owner.get(p_rid);
		ERR_FAIL_COND(!particles);

		particles_owner.free(p_rid);
		memdelete(particles);
	} else if (immediate_owner.owns(p_rid)) {

		Immediate *immediate = immediate_owner.get(p_rid);
		ERR_FAIL_COND(!immediate);

		immediate_owner.free(p_rid);
		memdelete(immediate);
	} else if (particles_instance_owner.owns(p_rid)) {

		ParticlesInstance *particles_isntance = particles_instance_owner.get(p_rid);
		ERR_FAIL_COND(!particles_isntance);

		particles_instance_owner.free(p_rid);
		memdelete(particles_isntance);

	} else if (skeleton_owner.owns(p_rid)) {

		Skeleton *skeleton = skeleton_owner.get( p_rid );
		ERR_FAIL_COND(!skeleton)

		skeleton_owner.free(p_rid);
		memdelete(skeleton);

	} else if (light_owner.owns(p_rid)) {

		Light *light = light_owner.get( p_rid );
		ERR_FAIL_COND(!light)

		light_owner.free(p_rid);
		memdelete(light);

	} else if (light_instance_owner.owns(p_rid)) {

		LightInstance *light_instance = light_instance_owner.get( p_rid );
		ERR_FAIL_COND(!light_instance);
		light_instance->clear_shadow_buffers();
		light_instance_owner.free(p_rid);
		memdelete( light_instance );

	} else if (fx_owner.owns(p_rid)) {

		FX *fx = fx_owner.get( p_rid );
		ERR_FAIL_COND(!fx);

		fx_owner.free(p_rid);
		memdelete( fx );

	} else if (environment_owner.owns(p_rid)) {

		Environment *env = environment_owner.get( p_rid );
		ERR_FAIL_COND(!env);

		environment_owner.free(p_rid);
		memdelete( env );
	} else if (sampled_light_owner.owns(p_rid)) {

		SampledLight *sampled_light = sampled_light_owner.get( p_rid );
		ERR_FAIL_COND(!sampled_light);

		sampled_light_owner.free(p_rid);
		memdelete( sampled_light );
	};
}


void RasterizerWIIU::custom_shade_model_set_shader(int p_model, RID p_shader) {


};

RID RasterizerWIIU::custom_shade_model_get_shader(int p_model) const {

	return RID();
};

void RasterizerWIIU::custom_shade_model_set_name(int p_model, const String& p_name) {

};

String RasterizerWIIU::custom_shade_model_get_name(int p_model) const {

	return String();
};

void RasterizerWIIU::custom_shade_model_set_param_info(int p_model, const List<PropertyInfo>& p_info) {

};

void RasterizerWIIU::custom_shade_model_get_param_info(int p_model, List<PropertyInfo>* p_info) const {

};


void RasterizerWIIU::ShadowBuffer::init(int p_size) {


#if 0
	size=p_size;

	//glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &depth);
	ERR_FAIL_COND(depth==0);

	/* Setup Depth Texture */
	glBindTexture(GL_TEXTURE_2D, depth);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, p_size, p_size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border_color[]={1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	/* Create FBO */
	glGenFramebuffers(1, &fbo);

	ERR_FAIL_COND( fbo==0 );

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	glDrawBuffer(GL_FALSE);
	glReadBuffer(GL_FALSE);

	/* Check FBO creation */
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);

	ERR_FAIL_COND( status==GL_FRAMEBUFFER_UNSUPPORTED );

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
#endif

}

void RasterizerWIIU::_init_shadow_buffers() {

	int near_shadow_size=GLOBAL_DEF("rasterizer/near_shadow_size",512);
	int far_shadow_size=GLOBAL_DEF("rasterizer/far_shadow_size",64);

	near_shadow_buffers.resize( GLOBAL_DEF("rasterizer/near_shadow_count",4) );
	far_shadow_buffers.resize( GLOBAL_DEF("rasterizer/far_shadow_count",16) );

	shadow_near_far_split_size_ratio = GLOBAL_DEF("rasterizer/shadow_near_far_split_size_ratio",0.3);

	for (int i=0;i<near_shadow_buffers.size();i++) {

		near_shadow_buffers[i].init(near_shadow_size );
	}

	for (int i=0;i<far_shadow_buffers.size();i++) {

		far_shadow_buffers[i].init(far_shadow_size);
	}

}


void RasterizerWIIU::_update_framebuffer() {

	return;

#if 0
	bool want_16 = GLOBAL_DEF("rasterizer/support_hdr",true);
	int blur_buffer_div=GLOBAL_DEF("rasterizer/blur_buffer_div",4);
	bool use_fbo = GLOBAL_DEF("rasterizer/use_fbo",true);


	if (blur_buffer_div<1)
		blur_buffer_div=2;


	if (use_fbo==framebuffer.active && framebuffer.width==window_size.width && framebuffer.height==window_size.height && framebuffer.buff16==want_16)
		return; //nuthin to change

	if (framebuffer.fbo!=0) {

		WARN_PRINT("Resizing the screen multiple times while using to FBOs may decrease performance on some hardware.");
		//free the framebuffarz
		glDeleteRenderbuffers(1,&framebuffer.fbo);
		glDeleteTextures(1,&framebuffer.depth);
		glDeleteTextures(1,&framebuffer.color);
		for(int i=0;i<2;i++) {
			glDeleteRenderbuffers(1,&framebuffer.blur[i].fbo);
			glDeleteTextures(1,&framebuffer.blur[i].color);

		}

		framebuffer.fbo=0;
	}

	framebuffer.active=use_fbo;
	framebuffer.width=window_size.width;
	framebuffer.height=window_size.height;
	framebuffer.buff16=want_16;


	if (!use_fbo)
		return;


	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	print_line("generating fbo, id: "+itos(framebuffer.fbo));
	//depth
	glGenTextures(1, &framebuffer.depth);

	glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,  framebuffer.width, framebuffer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);
	//color
	glGenTextures(1, &framebuffer.color);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);
	glTexImage2D(GL_TEXTURE_2D, 0, want_16?GL_RGB16F:GL_RGBA8,  framebuffer.width, framebuffer.height, 0, GL_RGBA, want_16?GL_HALF_FLOAT:GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ERR_FAIL_COND( status != GL_FRAMEBUFFER_COMPLETE );

	for(int i=0;i<2;i++) {

		glGenFramebuffers(1, &framebuffer.blur[i].fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.blur[i].fbo);

		glGenTextures(1, &framebuffer.blur[i].color);
		glBindTexture(GL_TEXTURE_2D, framebuffer.blur[i].color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  framebuffer.width/blur_buffer_div, framebuffer.height/blur_buffer_div, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.blur[i].color, 0);

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ERR_FAIL_COND( status != GL_FRAMEBUFFER_COMPLETE );
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void RasterizerWIIU::init() {

#ifdef GLES_OVER_GL
	glewInit();
#endif

    std::string errorLog(1024, '\0');
    GX2VertexShader *vertexShader = GLSL_CompileVertexShader(s_textureVertexShader, (char*)errorLog.data(), (int) errorLog.size(), GLSL_COMPILER_FLAG_NONE);
    if (!vertexShader) {
        OSReport("Vertex shader compilation failed for texture example: %s", errorLog.data());
        return;
    }
    GX2PixelShader *pixelShader = GLSL_CompilePixelShader(s_texturePixelShader, (char*)errorLog.data(), (int) errorLog.size(), GLSL_COMPILER_FLAG_NONE);
    if (!pixelShader) {
        OSReport("Pixel shader compilation failed for texture example: %s", errorLog.data());
        return;
    }

    memset(&shaderGroup, 0, sizeof(WHBGfxShaderGroup));
    shaderGroup.vertexShader = vertexShader;
    shaderGroup.pixelShader = pixelShader;
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, shaderGroup.vertexShader->program, shaderGroup.vertexShader->size);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, shaderGroup.pixelShader->program, shaderGroup.pixelShader->size);

    GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);

    /*WHBGfxInitShaderAttribute(&shaderGroup, "vertex", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32);
    WHBGfxInitShaderAttribute(&shaderGroup, "color_attrib", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32);
	WHBGfxInitShaderAttribute(&shaderGroup, "uv_attrib", 2, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);*/
	WHBGfxInitShaderAttribute(&shaderGroup, "aPos", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
    WHBGfxInitShaderAttribute(&shaderGroup, "aTexCoord", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
    WHBGfxInitFetchShader(&shaderGroup);


	scene_pass=1;
// 	if (ContextGL::get_singleton())
// 		ContextGL::get_singleton()->make_current();



	Set<String> extensions;
	// Vector<String> strings = String((const char*)glGetString( GL_EXTENSIONS )).split(" ",false);
	// for(int i=0;i<strings.size();i++) {

		// extensions.insert(strings[i]);
//		print_line(strings[i]);
	// }



	GLint tmp = 0;
//	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
//	print_line("GL_MAX_VERTEX_ATTRIBS "+itos(tmp));

	// glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LEQUAL);
	// glFrontFace(GL_CW);
	//glEnable(GL_TEXTURE_2D);

	default_material=create_default_material();

	_init_shadow_buffers();

	shadow=NULL;
	shadow_pass=0;

	framebuffer.fbo=0;
	framebuffer.width=0;
	framebuffer.height=0;
	framebuffer.buff16=false;
	framebuffer.blur[0].fbo=false;
	framebuffer.blur[1].fbo=false;
	framebuffer.active=false;

	//do a single initial clear
	// glClearColor(0,0,0,1);
	//glClearDepth(1.0);
	// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	skinned_buffer_size = GLOBAL_DEF("rasterizer/skinned_buffer_size",DEFAULT_SKINNED_BUFFER_SIZE);
	skinned_buffer = memnew_arr( uint8_t, skinned_buffer_size );

	// glGenTextures(1, &white_tex);
	unsigned char whitetexdata[8*8*3];
	for(int i=0;i<8*8*3;i++) {
		whitetexdata[i]=255;
	}
	//glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D,white_tex);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE,whitetexdata);

	npo2_textures_available=true;
	pvr_supported=extensions.has("GL_IMG_texture_compression_pvrtc");
	etc_supported=true;
	s3tc_supported=false;
	_rinfo.texture_mem=0;

	OSReport("Init shaders: OK\n");

}

void RasterizerWIIU::finish() {

	memdelete(skinned_buffer);
}

int RasterizerWIIU::get_render_info(VS::RenderInfo p_info) {

	switch(p_info) {

		case VS::INFO_OBJECTS_IN_FRAME: {

			return _rinfo.object_count;
		} break;
		case VS::INFO_VERTICES_IN_FRAME: {

			return _rinfo.vertex_count;
		} break;
		case VS::INFO_MATERIAL_CHANGES_IN_FRAME: {

			return _rinfo.mat_change_count;
		} break;
		case VS::INFO_SHADER_CHANGES_IN_FRAME: {

			return _rinfo.shader_change_count;
		} break;
		case VS::INFO_USAGE_VIDEO_MEM_TOTAL: {

			return 0;
		} break;
		case VS::INFO_VIDEO_MEM_USED: {

			return get_render_info(VS::INFO_TEXTURE_MEM_USED)+get_render_info(VS::INFO_VERTEX_MEM_USED);
		} break;
		case VS::INFO_TEXTURE_MEM_USED: {

			_rinfo.texture_mem;
		} break;
		case VS::INFO_VERTEX_MEM_USED: {

			return 0;
		} break;
	}

	return false;
}

bool RasterizerWIIU::needs_to_draw_next_frame() const {

	return false;
}

void RasterizerWIIU::reload_vram() {

	// glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LEQUAL);
	// glFrontFace(GL_CW);

	//do a single initial clear
	// glClearColor(0,0,0,1);
	//glClearDepth(1.0);
	// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

/*
	glGenTextures(1, &white_tex);
	unsigned char whitetexdata[8*8*3];
	for(int i=0;i<8*8*3;i++) {
		whitetexdata[i]=255;
	}
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,white_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE,whitetexdata);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0);

*/
	// glEnable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);

	List<RID> textures;
	texture_owner.get_owned_list(&textures);
	keep_copies=false;
	for(List<RID>::Element *E=textures.front();E;E=E->next()) {

		RID tid = E->get();
		Texture *t=texture_owner.get(tid);
		ERR_CONTINUE(!t);
		// t->tex_id=0;
		t->data_size=0;
		// glGenTextures(1, &t->tex_id);
		t->active=false;
		texture_allocate(tid,t->width,t->height,t->format,t->flags);
		bool had_image=false;
		for(int i=0;i<6;i++) {
			if (!t->image[i].empty()) {
				texture_set_data(tid,t->image[i],VS::CubeMapSide(i));
				had_image=true;
			}
		}

		if (!had_image && t->reloader) {
			Object *rl = ObjectDB::get_instance(t->reloader);
			if (rl)
				rl->call(t->reloader_func,tid);
		}
	}

	keep_copies=true;


}

bool RasterizerWIIU::has_feature(VS::Features p_feature) const {

	switch( p_feature) {
		case VS::FEATURE_SHADERS: return false;
		case VS::FEATURE_NEEDS_RELOAD_HOOK: return use_reload_hooks;
		default: return false;

	}

}


RasterizerWIIU::RasterizerWIIU(bool p_keep_copies,bool p_use_reload_hooks) {
	keep_copies=p_keep_copies;
	pack_arrays=false;
	use_reload_hooks=p_use_reload_hooks;

	frame = 0;

	skel_default.resize(1024 * 4);
	for (int i = 0; i < 1024 / 3; i++) {

		float *ptr = skel_default.ptr();
		ptr += i * 4 * 4;
		ptr[0] = 1.0;
		ptr[1] = 0.0;
		ptr[2] = 0.0;
		ptr[3] = 0.0;

		ptr[4] = 0.0;
		ptr[5] = 1.0;
		ptr[6] = 0.0;
		ptr[7] = 0.0;

		ptr[8] = 0.0;
		ptr[9] = 0.0;
		ptr[10] = 1.0;
		ptr[12] = 0.0;
	}

};

RasterizerWIIU::~RasterizerWIIU() {

};

#endif
