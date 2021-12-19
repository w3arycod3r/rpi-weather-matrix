#include "GameObjects.h"


void GameWorld::draw(FrameCanvas* buffer, float alpha)
{
    // Draw each drawable object
}

GameWorld::GameWorld()
{
    // Create each required object
}

void GameWorld::input(InputEvent* event)
{
    // Pass event to each updateable object
}

void GameWorld::update()
{
    // Update each updateable object
}

GameWorld::~GameWorld()
{
    // Free DMA for all the objects
}






void InputIndicator::input(InputEvent* event)
{
    if (event->isKey() && !event->isAutoRepeat())
    {
        uint32_t mask = 1 << (event->ie.code);

        if (event->ie.value) // Set Bit
            keyboardState |= mask;
        else // Clear Bit
            keyboardState &= ~mask;

        return;
    }
    if (event->isKeyboardEvent())
        return; // Done handling keyboard events


    // At this point, we are handling joystick events
	if (event->isAxis())
	{
		switch(event->jse.number)
		{
        // Scale axis values to radius value
        case PS3_LEFT_STICK_X:
            sticks[0].x = round(event->jse.value * radius / MAX_AXES_VALUE);
            break;
        case PS3_LEFT_STICK_Y:
            sticks[0].y = round(event->jse.value * radius / MAX_AXES_VALUE);
            break;
        case PS3_RIGHT_STICK_X:
            sticks[1].x = round(event->jse.value * radius / MAX_AXES_VALUE);
            break;
        case PS3_RIGHT_STICK_Y:
            sticks[1].y = round(event->jse.value * radius / MAX_AXES_VALUE);
            break;
		}
	}

    if (event->isButton())
    {
        uint32_t mask = 1 << (event->jse.number);

        if (event->jse.value) // Set Bit
            buttonStates |= mask;
        else // Clear Bit
            buttonStates &= ~mask;
    }
}

void InputIndicator::draw(FrameCanvas* buffer)
{
	
    for (int i = 0; i < 2; i++)
    {
        buffer->SetPixel(centers[i].x, centers[i].y, 0xff, 0, 0);
        buffer->SetPixel(centers[i].x + sticks[i].x, centers[i].y + sticks[i].y, 0, 0, 0xff);
    }

    uint8_t red, green, blue;
    for (uint8_t bit = 0; bit < 17; bit++)
    {
        if (buttonStates & (1 << bit))
        {
            red = 0xff;
            green = 0;
            blue = 0xff;
        }
        else
        {
            red = 0;
            green = 0;
            blue = 0;
        }
        buffer->SetPixel(bit + pos.x, pos.y, red, green, blue);
    }

    for (uint8_t bit = 0; bit < 32; bit++)
    {
        if (keyboardState & (1 << bit))
        {
            red = 0;
            green = 0xff;
            blue = 0;
        }
        else
        {
            red = 0;
            green = 0;
            blue = 0;
        }
        buffer->SetPixel(bit + pos.x, size.height + pos.y, red, green, blue);
    }
}