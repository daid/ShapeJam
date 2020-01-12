#include "mainScene.h"
#include "main.h"
#include "item.h"
#include "recipe.h"
#include "world.h"
#include "building.h"

#include <sp2/scene/camera.h>
#include <sp2/scene/voxelmap.h>
#include <sp2/graphics/meshbuilder.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/collision/3d/box.h>
#include <sp2/graphics/gui/loader.h>

#include "twitch.h"



Scene::Scene()
: sp::Scene("MAIN")
{
    ItemType::init();
    Recipe::init();

    camera_view_target = new sp::Node(getRoot());
    sp::P<sp::Camera> camera = new sp::Camera(camera_view_target);
    setDefaultCamera(camera);
    camera->setPerspective(sp::Vector2d(60, 60));
    camera->setPosition(sp::Vector3d(0, 0, 30));

    //new TwitchTest(getRoot());
    sp::P<World> world = new World(getRoot());

    gui = sp::gui::Loader::load("gui/hud.gui", "HUD");

    sp::P<sp::gui::Widget> row = sp::gui::Loader::load("gui/hud.gui", "INVENTORY_ROW", gui->getWidgetWithID("INVENTORY"));
    for(size_t n=0; n<inventory.size(); n++)
    {
        if (n == inventory.size() / 2)
            row = sp::gui::Loader::load("gui/hud.gui", "INVENTORY_ROW", gui->getWidgetWithID("INVENTORY"));
        inventory[n].widget = sp::gui::Loader::load("gui/hud.gui", "INVENTORY_BUTTON", row);
        inventory[n].type = nullptr;
        inventory[n].amount = 0;

        inventory[n].widget->setEventCallback([this, n](sp::Variant v)
        {
            for(auto& inv : inventory)
                inv.widget->setAttribute("theme_data", "button");
            inventory[n].widget->setAttribute("theme_data", "button.selected");
            selected_inventory_index = n;
        });
    }

    addInventory(ItemType::get("1"), 10);
    addInventory(ItemType::get("2"), 10);
    addInventory(ItemType::get("3"), 10);
    addInventory(ItemType::get("MINER"), 10);
    addInventory(ItemType::get("BELT"), 100);
    addInventory(ItemType::get("SPLITTER"), 10);
    addInventory(ItemType::get("FACTORY"), 10);
}

bool Scene::onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id)
{
    if (button == sp::io::Pointer::Button::Right)
    {
        pointer_action = PointerAction::Pickup;
        startPickup(getTileFromRay(ray));
    }
    else
    {
        stopPickup();
        pointer_action = PointerAction::SelectPlaceOrRotateView;
    }
    pointer_position = getCamera()->getGlobalTransform().inverse() * sp::Vector3f(ray.start);
    return true;
}

void Scene::onPointerDrag(sp::Ray3d ray, int id)
{
    sp::Vector3f p = getCamera()->getGlobalTransform().inverse() * sp::Vector3f(ray.start);

    sp::Vector3f diff = p - pointer_position;
    if (diff.length() > 0.1)
    {
        if (pointer_action == PointerAction::SelectPlaceOrRotateView)
            pointer_action = PointerAction::RotateView;
    }
    if (pointer_action == PointerAction::RotateView)
    {
        if (allow_rotate)
            camera_view_target->setRotation(camera_view_target->getRotation3D() * sp::Quaterniond::fromAxisAngle(sp::Vector3d(diff.y, -diff.x, 0), 3));
        pointer_position = p;
    }
    if (pointer_action == PointerAction::Pickup)
    {
        startPickup(getTileFromRay(ray));
    }
}

void Scene::onPointerUp(sp::Ray3d ray, int id)
{
    stopPickup();

    if (pointer_action == PointerAction::SelectPlaceOrRotateView)
    {
        queryCollisionAll(ray, [this](sp::P<sp::Node> obj, sp::Vector3d hit_location, sp::Vector3d hit_normal)
        {
            sp::P<World> world = obj;
            if (world)
            {
                Tile& tile = world->getTileAt(hit_location, hit_normal);

                if (selected_inventory_index > -1)
                {
                    auto& inv = inventory[selected_inventory_index];
                    
                    if (inv.amount > 0 && inv.type)
                    {
                        sp::P<sp::Node> result = inv.type->placeAt(world, hit_location, hit_normal);
                        if (result)
                        {
                            sp::P<Building> building = result;
                            if (building)
                                building->setDirection(new_placement_direction);
                            removeInventory(*inv.type, 1);
                            return false;
                        }
                    }
                }

                if (tile.building)
                {
                    tile.building->userRotate();
                    new_placement_direction = tile.building->getDirection();
                }
            }
            return false;
        });
    }
}

void Scene::onUpdate(float delta)
{
    if (escape_key.getUp())
    {
        delete this;
        openMainMenu();
        return;
    }
    if (pickup_timer.isRunning())
    {
        sp::MeshBuilder b;
        float f = pickup_timer.getProgress() * 2.0f - 1.0f;
        b.addQuad(sp::Vector3f(-1, 0.5, 0), sp::Vector3f(f, 0.5, 0), sp::Vector3f(-1, 0.8, 0), sp::Vector3f(f, 0.8, 0));
        pickup_indicator->render_data.mesh = b.create();

        if (pickup_timer.isExpired())
        {
            if (pickup_tile->item)
            {
                addInventory(pickup_tile->item->getType(), 1);
                pickup_tile->item.destroy();
            }
            else if (pickup_tile->building)
            {
                addInventory(*pickup_tile->building->placed_from_type, 1);
                pickup_tile->building.destroy();
            }
            else if (pickup_tile->getMineType())
            {
                addInventory(*pickup_tile->getMineType(), 1);
            }
            pickup_indicator.destroy();
            startPickup(pickup_tile);
        }
    }
}

void Scene::startPickup(Tile* tile)
{
    stopPickup();
    pickup_tile = tile;
    if (pickup_tile && (pickup_tile->item || pickup_tile->building || pickup_tile->getMineType()))
    {
        pickup_timer.start(0.7);

        pickup_indicator = new sp::Node(&pickup_tile->side.world);
        if (pickup_tile->building)
            pickup_indicator->setPosition(pickup_tile->building->getPosition3D());
        else
            pickup_indicator->setPosition(pickup_tile->position + pickup_tile->side.up);
        pickup_indicator->render_data.type = sp::RenderData::Type::Normal;
        pickup_indicator->render_data.shader = sp::Shader::get("shader/billboard.shader");
        pickup_indicator->render_data.texture = sp::texture_manager.get("gui/theme/pixel.png");
    }
}

void Scene::stopPickup()
{
    pickup_indicator.destroy();
    pickup_timer.stop();
}

Tile* Scene::getTileFromRay(sp::Ray3d ray)
{
    Tile* result = nullptr;
    queryCollisionAll(ray, [&result](sp::P<sp::Node> obj, sp::Vector3d hit_location, sp::Vector3d hit_normal)
    {
        sp::P<World> world = obj;
        if (world)
        {
            result = &world->getTileAt(hit_location, hit_normal);
        }
        return false;
    });
    return result;
}

void Scene::addInventory(ItemType& type, int amount)
{
    for(auto& entry : inventory)
    {
        if (entry.type == &type)
        {
            entry.amount += amount;
            entry.widget->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(entry.amount));
            return;
        }
    }

    for(auto& entry : inventory)
    {
        if (entry.type == nullptr)
        {
            entry.type = &type;
            entry.amount = amount;
            entry.widget->getWidgetWithID("IMAGE")->setAttribute("image", type.texture);
            entry.widget->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(entry.amount));
            return;
        }
    }
}

int Scene::removeInventory(ItemType& type, int amount)
{
    for(auto& entry : inventory)
    {
        if (entry.type == &type)
        {
            amount = std::min(entry.amount, amount);
            entry.amount -= amount;
            if (entry.amount < 1)
            {
                entry.type = nullptr;
                entry.widget->getWidgetWithID("IMAGE")->setAttribute("image", "");
                entry.widget->getWidgetWithID("AMOUNT")->setAttribute("caption", "");
            }
            else
            {
                entry.widget->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(entry.amount));
            }
            return amount;
        }
    }
    return 0;
}