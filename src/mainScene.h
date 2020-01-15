#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/timer.h>

#include "itemType.h"
#include "world.h"

class Scene : public sp::Scene
{
public:
    Scene();
    ~Scene();

    virtual bool onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) override;
    virtual void onPointerDrag(sp::Ray3d ray, int id) override;
    virtual void onPointerUp(sp::Ray3d ray, int id) override;
    virtual void onUpdate(float delta) override;

    void addInventory(const ItemType* type, int amount);
    int removeInventory(const ItemType* type, int amount);
private:
    Tile* getTileFromRay(sp::Ray3d ray);
    void startPickup(Tile* tile);
    void stopPickup();

    void setSelection(sp::P<Building> building);

    bool allow_rotate = false;
    enum class PointerAction
    {
        SelectPlaceOrRotateView,
        RotateView,
        Pickup
    } pointer_action;
    
    Direction new_placement_direction = Direction::Forward;
    sp::Vector3f pointer_position;
    sp::P<sp::Node> camera_view_target;

    sp::P<sp::gui::Widget> gui;

    class Inventory
    {
    public:
        sp::P<sp::gui::Widget> widget;
        const ItemType* type;
        int amount;
    };
    std::array<Inventory, 20> inventory;
    int selected_inventory_index = -1;

    sp::Timer pickup_timer;
    Tile* pickup_tile;
    sp::P<sp::Node> pickup_indicator;

    sp::P<Building> selected_building;
    sp::P<sp::Node> selection_indicator;
};

#endif//MAIN_SCENE_H
