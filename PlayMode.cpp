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
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
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

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	{
		for (auto &transform : scene.transforms) {
			if (transform.name == "Hip.FL") hip = &transform;
			else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
			else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
		}
		if (hip == nullptr) throw std::runtime_error("Hip not found.");
		if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
		if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

		hip_base_rotation = hip->rotation;
		upper_leg_base_rotation = upper_leg->rotation;
		lower_leg_base_rotation = lower_leg->rotation;

		//get pointer to camera for convenience:
		if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
		camera = &scene.cameras.front();

		//start music loop playing:
		// (note: position will be over-ridden in update())
		leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
	}
	Text text1;
	text1.text = "wowwkekjlwkjflkjwlkf";
    text1.start_pos = glm::vec2();
	text1.line_length = 100;
	text1.line_height = 100;

	texts.push_back(text1);

	Roboto = std::make_shared<Font> (data_path("Roboto/Roboto-Regular.ttf"), 72, 1280, 720);
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
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// key movements
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
	}

	return false;
}

void PlayMode::update(float elapsed) {
	// wobble leg
	{
		//slowly rotates through [0,1):
		wobble += elapsed / 10.0f;
		wobble -= std::floor(wobble);

		hip->rotation = hip_base_rotation * glm::angleAxis(
			glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
			glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
			glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);

		//move sound to follow leg tip position:
		leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);
	}
	//move camera:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	{ //reset button press counters:
		left.downs = 0;
		right.downs = 0;
		up.downs = 0;
		down.downs = 0;
	}
	/*
	tex_example.CLIP_FROM_LOCAL = camera->make_projection() * glm::mat4(camera->transform->make_world_to_local()) * glm::mat4(
		5.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 5.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 5.0f, 0.0f,
		0.0f, 0.0f, 20.0f, 1.0f
	);*/
	{ //---------- texture example generate some vertices ----------
		std::vector< PosTexVertex > verts;

		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(0.0f, 0.0f, 0.0f),
			.TexCoord = glm::vec2(0.0f, 0.0f),
		});
		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(0.0f, 1.0f, 0.0f),
			.TexCoord = glm::vec2(0.0f, 1.0f),
		});
		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(1.0f, 0.0f, 0.0f),
			.TexCoord = glm::vec2(1.0f, 0.0f),
		});
		verts.emplace_back(PosTexVertex{
			.Position = glm::vec3(1.0f, 1.0f, 0.0f),
			.TexCoord = glm::vec2(1.0f, 1.0f),
		});

		glBindTexture(GL_ARRAY_BUFFER, tex_example.tex);
		glBindBuffer(GL_ARRAY_BUFFER, tex_example.vbo);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		tex_example.count = verts.size();

		GL_ERRORS();

		//identity transform (just drawing "on the screen"):
		tex_example.CLIP_FROM_LOCAL = glm::mat4(1.0f);
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	{ //update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
		glUseProgram(lit_color_texture_program->program);
		glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
		glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
		glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
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

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
