#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"

#include <glm/gtc/type_ptr.hpp>

// LOADING MESHES
GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
  MeshBuffer const *ret = new MeshBuffer(data_path("hcube.pnct"));
  hexapod_meshes_for_lit_color_texture_program =
      ret->make_vao_for_program(lit_color_texture_program->program);
  return ret;
});
GLuint enemy_meshes_for_lit_color_texture_program = 1;
Load<MeshBuffer> enemy_meshes(LoadTagDefault, []() -> MeshBuffer const * {
  MeshBuffer const *ret = new MeshBuffer(data_path("enemy.pnct"));
  enemy_meshes_for_lit_color_texture_program =
      ret->make_vao_for_program(lit_color_texture_program->program);
  return ret;
});

// LOADING SCENES
Load<Scene> hexapod_scene(LoadTagDefault, []() -> Scene const * {
  return new Scene(data_path("hcube.scene"), [&](Scene &scene,
                                                 Scene::Transform *transform,
                                                 std::string const &mesh_name) {
    Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

    scene.drawables.emplace_back(transform);
    Scene::Drawable &drawable = scene.drawables.back();

    drawable.pipeline = lit_color_texture_program_pipeline;

    drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
    drawable.pipeline.type = mesh.type;
    drawable.pipeline.start = mesh.start;
    drawable.pipeline.count = mesh.count;
  });
});
Load<Scene> enemy_scene(LoadTagDefault, []() -> Scene const * {
  return new Scene(data_path("enemy.scene"), [&](Scene &scene,
                                                 Scene::Transform *transform,
                                                 std::string const &mesh_name) {
    Mesh const &mesh = enemy_meshes->lookup(mesh_name);
    scene.drawables.emplace_back(transform);
    Scene::Drawable &drawable = scene.drawables.back();

    drawable.pipeline = lit_color_texture_program_pipeline;

    drawable.pipeline.vao = enemy_meshes_for_lit_color_texture_program;
    drawable.pipeline.type = mesh.type;
    drawable.pipeline.start = mesh.start;
    drawable.pipeline.count = mesh.count;
  });
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
  // get pointer to camera for convenience:
  if (scene.cameras.size() != 1)
    throw std::runtime_error(
        "Expecting scene to have exactly one camera, but it has " +
        std::to_string(scene.cameras.size()));
  camera = &scene.cameras.front();

  // add enemy
  for (const Scene::Drawable &d : enemy_scene->drawables) {
    scene.drawables.push_back(d);
  }
}

PlayMode::~PlayMode() {}

bool PlayMode::handle_event(SDL_Event const &evt,
                            glm::uvec2 const &window_size) {

  if (evt.type == SDL_KEYDOWN) {
    if (evt.key.keysym.sym == SDLK_ESCAPE) {
      SDL_SetRelativeMouseMode(SDL_FALSE);
      return true;
    } else if (evt.key.keysym.sym == SDLK_a) {
      left.downs += 1;
      left.pressed = true;
      return true;
    } else if (evt.key.keysym.sym == SDLK_d) {
      right.downs += 1;
      right.pressed = true;
      return true;
    } else if (evt.key.keysym.sym == SDLK_w) {
      up.downs += 1;
      up.pressed = true;
      return true;
    } else if (evt.key.keysym.sym == SDLK_s) {
      down.downs += 1;
      down.pressed = true;
      return true;
    }
  } else if (evt.type == SDL_KEYUP) {
    if (evt.key.keysym.sym == SDLK_a) {
      left.pressed = false;
      return true;
    } else if (evt.key.keysym.sym == SDLK_d) {
      right.pressed = false;
      return true;
    } else if (evt.key.keysym.sym == SDLK_w) {
      up.pressed = false;
      return true;
    } else if (evt.key.keysym.sym == SDLK_s) {
      down.pressed = false;
      return true;
    }
  } else if (evt.type == SDL_MOUSEBUTTONDOWN) {
    if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
      SDL_SetRelativeMouseMode(SDL_TRUE);
      return true;
    }
    mouse.pressed = true;
  } else if (evt.type == SDL_MOUSEMOTION) {
    if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
      glm::vec2 motion = glm::vec2(evt.motion.xrel / float(window_size.y),
                                   evt.motion.yrel / float(window_size.y));

      camera->yaw += motion.x * camera->fovy;
      camera->pitch += motion.y * camera->fovy;
      // camera pitch should be limited to (-pi/2, pi/2)
      camera->pitch =
          glm::clamp(camera->pitch, -glm::pi<float>() / 2.0f + 0.0001f,
                     glm::pi<float>() / 2.0f - 0.0001f);

      // use it to get the new rotation, first by having lookAt
      glm::vec3 lookAt =
          glm::vec3(sin(camera->yaw) * cos(camera->pitch),
                    cos(camera->yaw) * cos(camera->pitch), sin(camera->pitch));
      glm::vec3 lookRight =
          glm::normalize(glm::vec3(-lookAt.y, lookAt.x, 0.0f));
      glm::vec3 lookUp = glm::cross(lookAt, lookRight);

      glm::mat3 rotationMatrix(lookRight, lookUp, lookAt);
      camera->transform->rotation = glm::quat_cast(rotationMatrix);

      return true;
    }
  }

  return false;
}

void PlayMode::update(float elapsed) {

  // move camera:
  {

    // combine inputs into a move:
    constexpr float PlayerSpeed = 30.0f;
    constexpr float EnemySpeed = 20.0f;
    constexpr float ShootbackSpeed = 480.0f;
    glm::vec2 move = glm::vec2(0.0f);
    if (left.pressed && !right.pressed)
      move.x = -1.0f;
    if (!left.pressed && right.pressed)
      move.x = 1.0f;
    if (down.pressed && !up.pressed)
      move.y = -1.0f;
    if (!down.pressed && up.pressed)
      move.y = 1.0f;

    // make it so that moving diagonally doesn't go faster:
    if (move != glm::vec2(0.0f))
      move = glm::normalize(move) * PlayerSpeed * elapsed;

    glm::mat4x3 frame = camera->transform->make_local_to_parent();
    glm::vec3 fright = frame[0];
    glm::vec3 fup = frame[1];
    glm::vec3 forward = -frame[2];

    glm::vec3 global_up = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 frame_forward = glm::cross(global_up, fright);

    glm::vec3 cur_pos = camera->transform->position;

    camera->transform->position += move.x * fright + move.y * frame_forward;
    // position's X and Y should be limited to (-100, 100)
    camera->transform->position.x =
        glm::clamp(camera->transform->position.x, -100.0f, 100.0f);
    camera->transform->position.y =
        glm::clamp(camera->transform->position.y, -100.0f, 100.0f);

    // move enemy
    glm::vec3 cur_enemy_pos = scene.drawables[enemy_index].transform->position;
    glm::vec3 enemy_dir = glm::normalize(cur_pos - cur_enemy_pos);
    scene.drawables[enemy_index].transform->position +=
        EnemySpeed * elapsed * enemy_dir;

    // check shoot angle difference, by checking -enemy_dir.xy and forward.xy
    glm::vec3 enemy_dir_xy =
        glm::normalize(glm::vec3(-enemy_dir.x, -enemy_dir.y, 0.0f));
    glm::vec3 forward_xy =
        glm::normalize(glm::vec3(forward.x, forward.y, 0.0f));
    float angle_diff = glm::acos(glm::dot(enemy_dir_xy, forward_xy));
    if (mouse.pressed) {
      // shoot
      if (angle_diff < glm::radians(30.0f)) {
        std::cout << "Hit" << std::endl;
        scene.drawables[enemy_index].transform->position -=
            ShootbackSpeed * elapsed * enemy_dir;
      }
    }
    mouse.pressed = false;
  }

  // reset button press counters:
  left.downs = 0;
  right.downs = 0;
  up.downs = 0;
  down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
  // update camera aspect ratio for drawable:
  camera->aspect = float(drawable_size.x) / float(drawable_size.y);

  // set up light type and position for lit_color_texture_program:
  // TODO: consider using the Light(s) in the scene to do this
  glUseProgram(lit_color_texture_program->program);
  glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
  glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1,
               glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
  glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1,
               glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
  glUseProgram(0);

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  glClearDepth(1.0f); // 1.0 is actually the default value to clear the depth
                      // buffer to, but FYI you can change it.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); // this is the default depth comparison function, but
                        // FYI you can change it.

  GL_ERRORS(); // print any errors produced by this setup code

  scene.draw(*camera);

  { // use DrawLines to overlay some text:
    glDisable(GL_DEPTH_TEST);
    float aspect = float(drawable_size.x) / float(drawable_size.y);
    DrawLines lines(glm::mat4(1.0f / aspect, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                              1.0f));

    constexpr float H = 0.09f;
    lines.draw_text(
        "Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
        glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
        glm::u8vec4(0x00, 0x00, 0x00, 0x00));
    float ofs = 2.0f / drawable_size.y;
    lines.draw_text(
        "Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
        glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
  }
}
