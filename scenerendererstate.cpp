#include "scenerendererstate.hpp"

#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>

#include "app.hpp"

namespace GameLib
{

SceneRendererState::SceneRendererState(App* app, Urho3D::Context* context) :
    UrhoExtras::States::State(context),
    app(app)
{
}

App* SceneRendererState::getApp()
{
    return app;
}

void SceneRendererState::prepareSceneForRendering()
{
    Urho3D::Renderer* renderer = GetSubsystem<Urho3D::Renderer>();

    // Create viewport
    Urho3D::Node* camera_node = app->getScene()->GetChild("camera");
    Urho3D::SharedPtr<Urho3D::Viewport> viewport(new Urho3D::Viewport(context_, app->getScene(), camera_node->GetComponent<Urho3D::Camera>()));
    renderer->SetViewport(0, viewport);

    // Set fog and ambient lighting
    Urho3D::Zone* default_zone = renderer->GetDefaultZone();
    default_zone->SetFogStart(app->getFogStartDistance());
    default_zone->SetFogEnd(app->getFogEndDistance());
    default_zone->SetFogColor(app->getFogColor());
    default_zone->SetAmbientColor(app->getAmbientLight());
}

}
