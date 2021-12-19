/*
 *  Title: GameObjects.h
 *  Author: Garrett Carter
 *  Date: 10/5/19
 *  Purpose: Base classes and structs used for game objects in any game.
 */

#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include <unistd.h>
#include <cmath>
#include "led-matrix.h"
#include "graphics.h"
#include <list>

#include "InputDevices.h"

using namespace rgb_matrix;
using std::list;

struct pos_int
{
    int x = 0;
    int y = 0;
};

struct pos_double
{
    double x = 0.0;
    double y = 0.0;
};

struct size_int
{
    int width = 0;
    int height = 0;
};

class GameObject
{
public:
    struct pos_double pos;
    struct size_int size;

};

// Drawable Object
class DGameObject: public GameObject
{
public:
    virtual void draw(FrameCanvas* buffer, float alpha) = 0;
};
// Updateable Object
class UGameObject: public GameObject
{
public:
    virtual void input(InputEvent* event) = 0;
    virtual void update() = 0;
};
// Drawable and Updateable
class DUGameObject: public GameObject
{
public:
    virtual void input(InputEvent* event) = 0;
    virtual void update() = 0;
    virtual void draw(FrameCanvas* buffer, float alpha) = 0;
};

class GameWorld
{
protected:
    list<DGameObject*> dObjects; // Doubly-Linked List
    list<UGameObject*> uObjects;
    list<DUGameObject*> duObjects;
public:
    void input(InputEvent* event);
    void update();
    void draw(FrameCanvas* buffer, float alpha); // Alpha needed for interpolation

    GameWorld(); // Create all objects
    ~GameWorld();
};

class InputIndicator
{
private:
    // Position of sticks in px, relative to center
    pos_int sticks[2];
    // Center points of stick indicators
    pos_int centers[2];
    // Radius -> Scaling of axis values
    double radius;

    uint32_t buttonStates = 0;
    uint32_t keyboardState = 0;

    // Top left of entire indicator display
    pos_int pos;
    // Size of entire indicator display
    size_int size;
    
public:
    InputIndicator(int w, int h, int x, int y)
    {
        size.width = w; size.height = h;
        pos.x = x; pos.y = y;

        // Left Stick Center
        centers[0].x = w/4 + x;
        centers[0].y = h/2 + y;
        // Right Stick Center
        centers[1].x = w/4 * 3 + x;
        centers[1].y = h/2 + y;

        radius = h/4.0;
    }
    void input(InputEvent* event);
    void update() { }
    void draw(FrameCanvas* buffer);
};

#endif