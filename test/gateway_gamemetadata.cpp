#include "catch2/catch.hpp"
#include "util.h"

#include "gateway.h"
#include "gateway_c.h"


// Generate an image 512x512 pixels, alternating between almost-black and debug pink.
// Generates an RGB image.
std::vector<uint8_t> generateImage()
{
	static uint32_t width = 512;
	static uint32_t height = 512;

	std::vector<uint8_t> result;
	result.resize(width * height * 3);

	for (uint32_t y = 0; y < height / 8; ++y)
	{
		for (uint32_t x = 0; x < width / 8; ++x)
		{
			// Compute parity for black or pink.
			uint32_t parity = (x + y) % 2;
			
			// Draw a 8x8 square.
			uint32_t upperLeftPixelIndex = 
				(y * 8) * width + 
				(x * 8);

			if (parity == 0)
			{
				for (uint32_t innerY = 0; innerY < 8; ++innerY)
				{
					for (uint32_t innerX = 0; innerX < 8; ++innerX)
					{
						uint32_t pixelIndex = upperLeftPixelIndex + innerY * width + innerX;
						result[pixelIndex * 3 + 0] = 0x05;
						result[pixelIndex * 3 + 1] = 0x05;
						result[pixelIndex * 3 + 2] = 0x05;
					}
				}
			}
			else if (parity == 1)
			{
				for (uint32_t innerY = 0; innerY < 8; ++innerY)
				{
					for (uint32_t innerX = 0; innerX < 8; ++innerX)
					{
						uint32_t pixelIndex = upperLeftPixelIndex + innerY * width + innerX;
						result[pixelIndex * 3 + 0] = 0x7F;
						result[pixelIndex * 3 + 1] = 0;
						result[pixelIndex * 3 + 2] = 0x7F;
					}
				}
			}
		}
	}

	return result;
}


TEST_CASE("Set basic image", "[gateawy][metadata]")
{
	gateway::SDK sdk("This is a game");
	std::vector<uint8_t> image = generateImage();
	
	gateway::GameMetadata metadata;
	metadata.GameLogo = gateway::EncodeImageToBase64PNG(image.data(), 512, 512, 3, 512 * 3, true);

	REQUIRE(metadata.GameLogo.size() > 0);
}