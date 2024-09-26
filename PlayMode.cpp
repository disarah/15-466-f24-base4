/**
 * Sections of texture rendering taken from Jim McCann's notes
 * With additional clarification help from Sasha's notes "Texture Rendering in OpenGL" on Jim's notes
 * Both of these were obtained via the class Discord
 */

#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "TextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("game4.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("game4.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
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

Load< Sound::Sample > game4_music_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("game4.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers for convenience:
	{
		for (auto &transform : scene.transforms) {
			if (transform.name == "Raccoon") raccoon = &transform;
			else if (transform.name == "Selector") selector = &transform;
		}
		if (raccoon == nullptr) throw std::runtime_error("Raccoon not found.");
		else if (selector == nullptr) throw std::runtime_error("Selector not found.");

		raccoon_rotation = raccoon->rotation;

		//get pointer to camera for convenience:
		if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
		camera = &scene.cameras.front();

		//start music loop playing:
		// (note: position will be over-ridden in update())
		music_loop = Sound::loop_3D(*game4_music_sample, 1.0f, raccoon->position, 10.0f);
	}

	Text text1(newline + "You are a Raccoon                                                                                     Press enter to Start",
				script_line_length,
				script_line_height
				);

	texts.push_back(text1);

	script = std::make_shared<Script> ();

	Roboto = std::make_shared<Font> (data_path("Roboto/Roboto-Regular.ttf"), font_size, font_width, font_height);
	Roboto->gen_texture(tex_example.tex, texts);

	{ //set up vertex array and buffers:
		glGenVertexArrays(1, &tex_example.vao);
		glGenBuffers(1, &tex_example.vbo);

		glBindVertexArray(tex_example.vao);
		glBindBuffer(GL_ARRAY_BUFFER, tex_example.vbo);

		glEnableVertexAttribArray( texture_program->Position_vec4 );
		glVertexAttribPointer( texture_program->Position_vec4, 3, GL_FLOAT, GL_FALSE, sizeof(PosTexVertex), (GLbyte *)0 + offsetof(PosTexVertex, Position) );

		glEnableVertexAttribArray( texture_program->TexCoord_vec2 );
		glVertexAttribPointer( texture_program->TexCoord_vec2, 2, GL_FLOAT, GL_FALSE, sizeof(PosTexVertex), (GLbyte *)0 + offsetof(PosTexVertex, TexCoord) );

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	GL_ERRORS();

	{ //---------- texture example generate some vertices ----------
		std::vector< PosTexVertex > verts;

		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(-0.5f, -0.8f, 0.0f),
			.TexCoord = glm::vec2(0.0f, 0.0f),
		});
		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(-0.5f, 0.7f, 0.0f),
			.TexCoord = glm::vec2(0.0f, 1.0f),
		});
		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(1.5f, -0.8f, 0.0f),
			.TexCoord = glm::vec2(1.0f, 0.0f),
		});
		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(1.5f, 0.7f, 0.0f),
			.TexCoord = glm::vec2(1.0f, 1.0f),
		});

		glBindBuffer(GL_ARRAY_BUFFER, tex_example.vbo);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		tex_example.count = verts.size();

		GL_ERRORS();

		//identity transform (just drawing "on the screen"):
		// slight scaling to make text smaller
		tex_example.CLIP_FROM_LOCAL = glm::mat4(
			0.8f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.05f, 0.0f, 0.0f, 1.0f
		);
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// key movements
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.downs += 1;
			enter.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.pressed = false;
			enterb = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			rightb = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			leftb = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	// wobble raccoon
	{
		//slowly rotates through [0,1):
		wobble += elapsed / 10.0f;
		wobble -= std::floor(wobble);

		raccoon->rotation = raccoon_rotation * glm::angleAxis(
			glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
	//move camera:
	{
		//combine inputs into a move:
		if (enter.pressed && !right.pressed && !left.pressed && !enterb){
			enterb = true;
			texts[0].text = script->get_next_line((uint32_t)std::round((selector->position.x + 0.4f)/0.7f));
			Roboto->gen_texture(tex_example.tex, texts);
		}
		if (!right.pressed && left.pressed && !leftb){
			leftb = true;
			selector->position.x = std::clamp(selector->position.x - 0.7f, -0.4f, 1.f);
		}
		if (right.pressed && !left.pressed && !rightb){
			rightb = true;
			selector->position.x = std::clamp(selector->position.x + 0.7f, -0.4f, 1.f);
		}
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	{ //reset button press counters:
		enter.downs = 0;
		right.downs = 0;
		left.downs = 0;
	}
	
	/*tex_example.CLIP_FROM_LOCAL = camera->make_projection() * glm::mat4(camera->transform->make_world_to_local()) * glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);*/
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	{ //update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
		glUseProgram(lit_color_texture_program->program);
		glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
		glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
		glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 0.8f, 0.6f)));
		glUseProgram(0);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

		scene.draw(*camera);
	}

	{ //texture example drawing
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(texture_program->program);
		glBindVertexArray(tex_example.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_example.tex);

		glUniformMatrix4fv( texture_program->CLIP_FROM_LOCAL_mat4, 1, GL_FALSE, glm::value_ptr(tex_example.CLIP_FROM_LOCAL) );

		glDrawArrays(GL_TRIANGLE_STRIP, 0, tex_example.count);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);

		glDisable(GL_BLEND);
		
	}
	GL_ERRORS();
}