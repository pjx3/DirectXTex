//-------------------------------------------------------------------------------------
// DirectXTexRAW.cpp
//  
// DirectX Texture Library - RAW file format reader/writer
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
//-------------------------------------------------------------------------------------

#include "directxtexp.h"

// This implementation currently only supports headerless R32G32B32A32_Float format
// RAW files in a little endian order.
// It relies on metadata given via command line arguments to determine width & height

using namespace DirectX;

//=====================================================================================
// Private utility functions
//=====================================================================================

namespace
{
	//-------------------------------------------------------------------------------------
	// Copies pixel data from a RAW into the target image
	//-------------------------------------------------------------------------------------
	HRESULT CopyPixels(
		_In_reads_bytes_(size) const void* pSource,
		size_t size,
		_In_ const Image* image)
	{
		if (!pSource || 
			!size ||
			!image || 
			!image->pixels) 
		{
			return E_POINTER;
		}

		auto src = reinterpret_cast<float const*>(pSource);
		auto dst = reinterpret_cast<float*>(image->pixels);

		for (size_t y = 0; y < image->height; y++) 
		{
			for (size_t x = 0; x < image->width; x++) 
			{
				float r = *src++;
				float g = *src++;
				float b = *src++;
				float a = *src++;
				(void)a;	// unused

				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = 1.0f;	// note: forcing alpha!
			}
		}

		return S_OK;
	}

} // namespace

//=====================================================================================
// Public entry-points
//=====================================================================================

//-------------------------------------------------------------------------------------
// Load a RAW file in memory
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT DirectX::LoadFromRAWMemory(
	const void* pSource,
	size_t size,
	TexMetadata* metadata,
	ScratchImage& image)
{
	if (!pSource || 
		size == 0 ||
		!metadata ||
		metadata->width == 0 ||
		metadata->height == 0 ||
		metadata->format != DXGI_FORMAT_R32G32B32A32_FLOAT)
	{
		return E_INVALIDARG;
	}

	image.Release();

	const void* pPixels = static_cast<const uint8_t*>(pSource);

	HRESULT hr = image.Initialize2D(metadata->format, metadata->width, metadata->height, 1, 1);
	if (FAILED(hr)) {
		return hr;
	}

	hr = CopyPixels(pPixels, size, image.GetImage(0, 0, 0));
	if (FAILED(hr))
	{
		image.Release();
		return hr;
	}

	return S_OK;
}

