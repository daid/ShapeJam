#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/script/environment.h>
#include <sp2/timer.h>

#include "itemType.h"
#include "world.h"

class Scene : public sp::Scene
{
public:
    Scene();
    ~Scene();

    virtual bool onPointerMove(sp::Ray3d ray, int id) override;
    virtual void onPointerLeave(int id) override;
    virtual bool onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) override;
    virtual void onPointerDrag(sp::Ray3d ray, int id) override;
    virtual void onPointerUp(sp::Ray3d ray, int id) override;
    virtual void onUpdate(float delta) override;
    virtual void onFixedUpdate() override;

    void addInventory(const ItemType* type, int amount);
    int countInventory(const ItemType* type);
    int removeInventory(const ItemType* type, int amount);

    void message(const sp::string& message);
    void objective(const sp::string& message);
    void setupMessageConfirm();
    void enableRotation();
private:
    Tile* getTileFromRay(sp::Ray3d ray, sp::Vector2i object_size=sp::Vector2i(1, 1));
    void startPickup(Tile* tile);
    void stopPickup();

    void setSelection(sp::P<Building> building);

    bool allow_rotate = false;
    enum class PointerAction
    {
        None,
        SelectPlaceOrRotateView,
        RotateView,
        Pickup
    } pointer_action = PointerAction::None;
    
    Direction new_placement_direction = Direction::Forward;
    sp::Vector3f pointer_position;
    sp::P<sp::Node> camera_view_target;

    sp::P<sp::gui::Widget> gui;
    sp::gui::Loader gui_loader{"gui/hud.gui"};

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

    sp::P<sp::Node> place_preview;

    sp::P<Building> selected_building;
    sp::P<sp::Node> selection_indicator;

    sp::P<sp::script::Environment> script_environment;
    sp::script::CoroutinePtr script_coroutine;

    sp::string script_message;
    size_t script_message_length = 0;
    bool script_blocked = false;
};

#endif//MAIN_SCENE_H
