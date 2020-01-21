#include "mainScene.h"
#include "main.h"
#include "item.h"
#include "recipe.h"
#include "world.h"
#include "building.h"
#include "stats.h"

#include <sp2/engine.h>
#include <sp2/scene/camera.h>
#include <sp2/scene/voxelmap.h>
#include <sp2/graphics/meshbuilder.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/collision/3d/box.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/audio/sound.h>

#include "twitch.h"


static int lua_Yield(lua_State* lua)
{
    return lua_yield(lua, 0);
}

static int lua_confirm(lua_State* lua)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    scene->setupMessageConfirm();
    return lua_yield(lua, 0);
}

static void lua_addInventory(sp::string item, int amount)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    scene->addInventory(ItemType::get(item), amount);
}

static int lua_countInventory(sp::string item)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    return scene->countInventory(ItemType::get(item));
}

static int lua_removeInventory(sp::string item, int amount)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    return scene->removeInventory(ItemType::get(item), amount);
}

static void lua_message(sp::string msg)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    scene->message(msg);
}

static void lua_objective(sp::string msg)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    scene->objective(msg);
}

static void lua_enableRotation()
{
    sp::P<Scene> scene = Scene::get("MAIN");
    scene->enableRotation();
}

static int lua_secondStats(sp::string itemType)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    for(sp::P<World> world : scene->getRoot()->getChildren())
    {
        if (world)
            return world->stats->totalForSecond(ItemType::get(itemType));
    }
    return 0;
}

static int lua_minuteStats(sp::string itemType)
{
    sp::P<Scene> scene = Scene::get("MAIN");
    for(sp::P<World> world : scene->getRoot()->getChildren())
    {
        if (world)
            return world->stats->totalForMinute(ItemType::get(itemType));
    }
    return 0;
}

Scene::Scene()
: sp::Scene("MAIN")
{
    ItemType::init();
    Recipe::init();
    ItemType::initRecipes();

    camera_view_target = new sp::Node(getRoot());
    sp::P<sp::Camera> camera = new sp::Camera(camera_view_target);
    setDefaultCamera(camera);
    camera->setPerspective(sp::Vector2d(60, 60));
    camera->setPosition(sp::Vector3d(0, 0, 30));

    //new TwitchTest(getRoot());
    sp::P<World> world = new World(getRoot());

    gui = gui_loader.create("HUD");

    sp::P<sp::gui::Widget> row = gui_loader.create("INVENTORY_ROW", gui->getWidgetWithID("INVENTORY"));
    for(size_t n=0; n<inventory.size(); n++)
    {
        if (n == inventory.size() / 2)
            row = gui_loader.create("INVENTORY_ROW", gui->getWidgetWithID("INVENTORY"));
        inventory[n].widget = gui_loader.create("INVENTORY_BUTTON", row);
        inventory[n].type = nullptr;
        inventory[n].amount = 0;

        inventory[n].widget->setEventCallback([this, n](sp::Variant v)
        {
            for(auto& inv : inventory)
                inv.widget->setAttribute("theme_data", "button");
            if (selected_inventory_index == int(n))
            {
                selected_inventory_index = -1;
            }
            else
            {
                inventory[n].widget->setAttribute("theme_data", "button.selected");
                selected_inventory_index = n;
            }
        });
    }
    gui->getWidgetWithID("MESSAGE_BUTTON")->setEventCallback([this](sp::Variant v)
    {
        if (script_message_length < script_message.length())
        {
            script_message_length = script_message.length();
            gui->getWidgetWithID("MESSAGE")->setAttribute("caption", script_message);
        }
        else
        {
            script_blocked = false;
            gui->getWidgetWithID("MESSAGE_BUTTON")->hide();
            gui->getWidgetWithID("MESSAGE_BOX")->hide();
        }
    });
    gui->getWidgetWithID("STATS_BUTTON")->setEventCallback([this](sp::Variant v)
    {
        gui->getWidgetWithID("STATS_PANEL")->setVisible(!gui->getWidgetWithID("STATS_PANEL")->isVisible());
    });

    script_environment = new sp::script::Environment();
    script_environment->load("scenario.lua");
    script_environment->setGlobal("yield", lua_Yield);
    script_environment->setGlobal("confirm", lua_confirm);
    script_environment->setGlobal("message", lua_message);
    script_environment->setGlobal("objective", lua_objective);
    script_environment->setGlobal("addInventory", lua_addInventory);
    script_environment->setGlobal("countInventory", lua_countInventory);
    script_environment->setGlobal("removeInventory", lua_removeInventory);
    script_environment->setGlobal("enableRotation", lua_enableRotation);
    script_environment->setGlobal("secondStats", lua_secondStats);
    script_environment->setGlobal("minuteStats", lua_minuteStats);
    script_coroutine = script_environment->callCoroutine("run");
}

Scene::~Scene()
{
    gui.destroy();
    script_coroutine = nullptr;
    script_environment.destroy();
}

bool Scene::onPointerMove(sp::Ray3d ray, int id)
{
    if (selected_inventory_index < 0 || inventory[selected_inventory_index].amount < 1)
        return false;

    sp::Vector2i size = inventory[selected_inventory_index].type->size;
    Tile* t = getTileFromRay(ray, size);
    if (t)
    {
        if (!place_preview)
        {
            place_preview = new sp::Node(&t->side.world);
            place_preview->render_data.type = sp::RenderData::Type::Transparent;
            place_preview->render_data.shader = sp::Shader::get("internal:color.shader");
            place_preview->render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(size));
            place_preview->render_data.color = sp::Color(0.0, 0.5, 0.0, 0.3);
        }
        place_preview->setPosition(t->position + t->side.up * 0.05 + t->side.right * ((size.x - 1) * 0.5) + t->side.forward * ((size.y - 1) * 0.5));
        place_preview->setRotation(t->side.rotation);
        return true;
    }
    return false;
}

void Scene::onPointerLeave(int id)
{
    place_preview.destroy();
}

bool Scene::onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id)
{
    camera_snap_timer.stop();

    if (button == sp::io::Pointer::Button::Right || (button == sp::io::Pointer::Button::Touch && selected_inventory_index == -1))
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
        Tile* tile = getTileFromRay(ray);
        if (tile != pickup_tile)
            startPickup(tile);
    }
}

void Scene::onPointerUp(sp::Ray3d ray, int id)
{
    stopPickup();

    if (pointer_action == PointerAction::SelectPlaceOrRotateView)
    {
        if (selected_inventory_index > -1 && inventory[selected_inventory_index].amount > 0 && inventory[selected_inventory_index].type)
        {
            Inventory& inv = inventory[selected_inventory_index];
            Tile* corner_tile = getTileFromRay(ray, inv.type->size);
            if (corner_tile)
            {
                sp::P<sp::Node> result = inv.type->placeAt(corner_tile);
                if (result)
                {
                    sp::P<Building> building = result;
                    if (building)
                        building->setDirection(new_placement_direction);
                    removeInventory(inv.type, 1);
                    setSelection(corner_tile->building);
                    return;
                }
            }
        }
        Tile* center_tile = getTileFromRay(ray);
        if (center_tile)
        {
            if (center_tile->building && selected_building == center_tile->building)
            {
                center_tile->building->userRotate();
                new_placement_direction = center_tile->building->getDirection();
            }
            setSelection(center_tile->building);
        }
    }
    if (pointer_action == PointerAction::RotateView)
    {
        auto rotation = camera_view_target->getRotation3D();
        auto forward = rotation * sp::Vector3d(0, 0, 1);

        for(auto v : {sp::Vector3d(1, 0, 0), sp::Vector3d(-1, 0, 0), sp::Vector3d(0, 1, 0), sp::Vector3d(0, -1, 0), sp::Vector3d(0, 0, 1), sp::Vector3d(0, 0, -1)})
        {
            if ((forward - v).length() < 0.35)
            {
                camera_rotate_source = rotation;
                camera_rotate_target = sp::Quaterniond::fromVectorToVector(forward, v) * rotation;
                camera_rotate_target.normalize();
                camera_snap_timer.start(0.5);
                
                sp::Vector3d world_up(v.y, v.z, v.x);
                sp::Vector3d source_vector = camera_rotate_target * sp::Vector3d(1, 0, 0);
                for(auto dir : {camera_rotate_target * sp::Vector3d(0, 1, 0), camera_rotate_target * sp::Vector3d(-1, 0, 0), camera_rotate_target * sp::Vector3d(0,-1, 0)})
                {
                    if ((world_up - dir).length() < (world_up - source_vector).length())
                        source_vector = dir;
                }
                camera_rotate_target = sp::Quaterniond::fromVectorToVector(source_vector, world_up) * camera_rotate_target;
            }
        }
    }
    pointer_action = PointerAction::None;
}

void Scene::onUpdate(float delta)
{
    if (escape_key.getUp())
    {
        disable();
        sp::Engine::getInstance()->setGameSpeed(0.0);
        sp::P<sp::gui::Widget> menu = sp::gui::Loader::load("gui/ingame_menu.gui", "INGAME_MENU");
        menu->getWidgetWithID("RESUME")->setEventCallback([=](sp::Variant v) mutable
        {
            menu.destroy();
            sp::Engine::getInstance()->setGameSpeed(1.0);
            enable();
        });
        menu->getWidgetWithID("EXIT")->setEventCallback([=](sp::Variant v) mutable
        {
            menu.destroy();
            sp::Engine::getInstance()->setGameSpeed(1.0);
            sp::Scene::get("MAIN").destroy();
            openMainMenu();
        });
        return;
    }
    if (!selected_building)
        gui->getWidgetWithID("INFO_PANEL")->hide();
    if (pickup_timer.isRunning())
    {
        sp::MeshBuilder b;
        float f = pickup_timer.getProgress() * 2.0f - 1.0f;
        b.addQuad(sp::Vector3f(-1, 0.5, 0), sp::Vector3f(f, 0.5, 0), sp::Vector3f(-1, 0.8, 0), sp::Vector3f(f, 0.8, 0));
        pickup_indicator->render_data.mesh = b.create();

        if (pickup_timer.isExpired())
        {
            sp::audio::Sound::play("pickup.wav");
            if (pickup_tile->item)
            {
                addInventory(pickup_tile->item->getType(), 1);
                pickup_tile->item.destroy();
            }
            else if (pickup_tile->building)
            {
                addInventory(pickup_tile->building->placed_from_type, 1);
                pickup_tile->building.destroy();
            }
            else if (pickup_tile->getMineType())
            {
                addInventory(pickup_tile->getMineType(), 1);
            }
            pickup_indicator.destroy();
            startPickup(pickup_tile);
        }
    }

    if (camera_snap_timer.isRunning())
    {
        if (camera_snap_timer.isExpired())
            camera_view_target->setRotation(camera_rotate_target);
        else
            camera_view_target->setRotation(camera_rotate_source.slerp(camera_rotate_target, camera_snap_timer.getProgress()));
    }
}

void Scene::onFixedUpdate()
{
    if (script_coroutine && !script_blocked && !script_coroutine->resume())
        script_coroutine = nullptr;
    
    if (script_message_length < script_message.length())
    {
        script_message_length += 1;
        gui->getWidgetWithID("MESSAGE_BOX")->show();
        gui->getWidgetWithID("MESSAGE")->setAttribute("caption", script_message.substr(0, script_message_length));
    }

    sp::P<sp::gui::Widget> stats_panel = gui->getWidgetWithID("STATS_PANEL");
    if (stats_panel->isVisible())
    {
        for(sp::P<World> world : getRoot()->getChildren())
        {
            if (world)
                world->stats->show(stats_panel->getWidgetWithID("STATS_CONTENTS"));
        }
    }

    if (selected_building)
    {
        sp::P<sp::gui::Widget> info = gui->getWidgetWithID("INFO_PANEL");

        auto box_iterator = info->getWidgetWithID("RECIPES")->getChildren().begin();
        for(auto recipe : selected_building->placed_from_type->recipes)
        {
            sp::P<sp::gui::Widget> recipe_box = *box_iterator;
            ++box_iterator;
            auto input_iterator = recipe_box->getWidgetWithID("INPUT")->getChildren().begin();
            for(auto input : recipe->input)
            {
                sp::P<sp::gui::Widget> recipe_item = *input_iterator;
                ++input_iterator;
                recipe_item->getWidgetWithID("PROGRESS")->layout.size.y = std::min(20.0, 20.0 * selected_building->getInventoryCount(input.first) / input.second);
            }
        }

    }
}

void Scene::startPickup(Tile* tile)
{
    stopPickup();
    pickup_tile = tile;
    if (pickup_tile && (pickup_tile->item || pickup_tile->building || pickup_tile->getMineType()))
    {
        pickup_timer.start(0.3);

        pickup_indicator = new sp::Node(&pickup_tile->side.world);
        if (pickup_tile->building)
            pickup_indicator->setPosition(pickup_tile->building->getPosition3D());
        else
            pickup_indicator->setPosition(pickup_tile->position + pickup_tile->side.up);
        pickup_indicator->render_data.type = sp::RenderData::Type::Normal;
        pickup_indicator->render_data.shader = sp::Shader::get("shader/billboard.shader");
        pickup_indicator->render_data.texture = sp::texture_manager.get("gui/theme/pixel.png");
        pickup_indicator->render_data.color = sp::Color(0.2, 1.0, 0.2);
    }
}

void Scene::stopPickup()
{
    pickup_indicator.destroy();
    pickup_timer.stop();
}

void Scene::setSelection(sp::P<Building> building)
{
    selected_building = building;
    selection_indicator.destroy();
    if (!building)
    {
        gui->getWidgetWithID("INFO_PANEL")->hide();
        return;
    }
    sp::P<sp::gui::Widget> info = gui->getWidgetWithID("INFO_PANEL");
    info->show();
    info->getWidgetWithID("NAME")->setAttribute("caption", building->placed_from_type->label);
    while(!info->getWidgetWithID("RECIPES")->getChildren().empty())
        delete **info->getWidgetWithID("RECIPES")->getChildren().begin();
    info->getWidgetWithID("RECIPES")->setSize(0, 0);

    for(auto recipe : building->placed_from_type->recipes)
    {
        sp::P<sp::gui::Widget> recipe_box = gui_loader.create("RECIPE_BOX", info->getWidgetWithID("RECIPES"));
        for(auto input : recipe->input)
        {
            sp::P<sp::gui::Widget> recipe_item = gui_loader.create("RECIPE_INPUT_ITEM", recipe_box->getWidgetWithID("INPUT"));
            recipe_item->getWidgetWithID("IMAGE")->setAttribute("image", input.first->texture);
            recipe_item->getWidgetWithID("PROGRESS")->layout.size.y = std::min(20.0, 20.0 * building->getInventoryCount(input.first) / input.second);
            recipe_item->getWidgetWithID("AMOUNT")->setAttribute("caption", "x" + sp::string(input.second));
        }
        for(auto output : recipe->output)
        {
            sp::P<sp::gui::Widget> recipe_item = gui_loader.create("RECIPE_OUTPUT_ITEM", recipe_box->getWidgetWithID("OUTPUT"));
            recipe_item->getWidgetWithID("IMAGE")->setAttribute("image", output.first->texture);
            recipe_item->getWidgetWithID("AMOUNT")->setAttribute("caption", "x" + sp::string(output.second));
        }
        if (selected_building->getRecipe() == recipe)
            recipe_box->setAttribute("theme_data", "button.selected");
        recipe_box->setEventCallback([this, recipe](sp::Variant v)
        {
            selected_building->setRecipe(recipe);
            setSelection(selected_building);
        });
    }

    selection_indicator = new sp::Node(selected_building);
    selection_indicator->setPosition(sp::Vector3d(0, 0, -0.05));
    selection_indicator->render_data.type = sp::RenderData::Type::Additive;
    selection_indicator->render_data.shader = sp::Shader::get("internal:color.shader");
    selection_indicator->render_data.texture = sp::texture_manager.get("gui/theme/pixel.png");
    selection_indicator->render_data.mesh = sp::MeshData::createDoubleSidedQuad(sp::Vector2f(selected_building->getSize()) + sp::Vector2f(0.2, 0.2));
    selection_indicator->render_data.color = sp::Color(0.4, 0.8, 0.4);
}

Tile* Scene::getTileFromRay(sp::Ray3d ray, sp::Vector2i object_size)
{
    Tile* result = nullptr;
    queryCollisionAll(ray, [&result, object_size](sp::P<sp::Node> obj, sp::Vector3d hit_location, sp::Vector3d hit_normal)
    {
        sp::P<World> world = obj;
        if (world)
        {
            WorldSide& side = world->getSideAt(hit_normal);
            hit_location -= side.right * ((object_size.x - 1) * 0.5);
            hit_location -= side.forward * ((object_size.y - 1) * 0.5);
            result = &side.getTileAt(hit_location);
        }
        return false;
    });
    return result;
}

void Scene::addInventory(const ItemType* type, int amount)
{
    if (!type)
        return;
    for(auto& entry : inventory)
    {
        if (entry.type == type)
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
            entry.type = type;
            entry.amount = amount;
            entry.widget->getWidgetWithID("IMAGE")->setAttribute("image", type->texture);
            entry.widget->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(entry.amount));
            return;
        }
    }
}

int Scene::countInventory(const ItemType* type)
{
    if (!type)
        return 0;
    for(auto& entry : inventory)
    {
        if (entry.type == type)
        {
            return entry.amount;
        }
    }
    return 0;
}

int Scene::removeInventory(const ItemType* type, int amount)
{
    if (!type)
        return 0;
    for(auto& entry : inventory)
    {
        if (entry.type == type)
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

void Scene::message(const sp::string& msg)
{
    script_message = ItemType::translate(msg);
    script_message_length = 0;
    if (script_message == "")
    {
        gui->getWidgetWithID("MESSAGE_BOX")->hide();
    }
}

void Scene::objective(const sp::string& message)
{
    if (message == "")
    {
        gui->getWidgetWithID("OBJECTIVE_BOX")->hide();
    }
    else
    {
        gui->getWidgetWithID("OBJECTIVE_BOX")->show();
        gui->getWidgetWithID("OBJECTIVE")->setAttribute("caption", ItemType::translate(message));
    }
}

void Scene::setupMessageConfirm()
{
    script_blocked = true;
    gui->getWidgetWithID("MESSAGE_BUTTON")->show();
}

void Scene::enableRotation()
{
    allow_rotate = true;
}
