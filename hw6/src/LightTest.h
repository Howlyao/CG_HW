#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <imgui.h>
#include <sources/imgui_impl_glfw.h>
#include <sources/imgui_impl_opengl3.h>
#include <iostream>
#include <Model/shader.h>
#include <Model/camera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

class LightTest {
private:	
	const unsigned int SCR_WIDTH = 1280;
	const unsigned int SCR_HEIGHT = 720;

	Shader phoneShader = Shader("shaders/hw6/phone.vs", "shaders/hw6/phone.fs");
	Shader gouraudShader = Shader("shaders/hw6/gouraud.vs", "shaders/hw6/gouraud.fs");
	//立方体和球体的顶点数组对象和缓冲对象
	unsigned int VAO = 0, VBO;
	unsigned int sphereVAO = 0, sphereVBO;
	int indexCount = 0;
	int radioBtn = 0;
	//ambient,diffuse,specular,反光度参数
	float ambient = 0.5f, diffuse = 0.5f, specular = 0.5, shininess = 32.0;
	//平行光方向参数
	float dirX = -0.6, dirY = 0.2, dirZ = 0.1;
	//点光源位置参数
	float posX = 3.0, posY = 3.0, posZ = 3.0;
	//平行光方向向量
	glm::vec3 lightDirection = glm::vec3(-1.0, -1.0, -1.0);
	//点光源位置向量
	glm::vec3 lightPos = glm::vec3(3.0, 3.0, 3.0);
public:


	void draw(Camera &camera) {
		glEnable(GL_DEPTH_TEST);
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Light Shading Test");
		//radiobtn
		//有5种模式，分别PhoneShading与GouraudShaing的平行光和点光源，以及移动光源
		ImGui::RadioButton("Phone Shading with direct light", &radioBtn, 0);//Phone模型演示 平行光
		ImGui::RadioButton("Phone Shading with point light", &radioBtn, 1);//Phone模型演示 点光源
		ImGui::RadioButton("Gouraud Shading with direct light", &radioBtn, 2);//Gouraud
		ImGui::RadioButton("Gouraud Shading with point light", &radioBtn, 3);
		ImGui::RadioButton("Phone with moving light", &radioBtn, 4);
		
		//GUI修改光源参数ambient,diffuse,specular以及物体的反光度
		ImGui::Text("The light parameters and the shininess of the object");
		ImGui::SliderFloat("ambient", &ambient, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", &diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &specular, 0.0f, 1.0f);
		ImGui::SliderFloat("shininess", &shininess, 0.1f, 100.0f);

		if (radioBtn == 1 || radioBtn == 3) {
			//GUI修改光源位置
			ImGui::SliderFloat("posX", &posX, -10.0f, 10.0f);
			ImGui::SliderFloat("posY", &posY, -10.0f, 10.0f);
			ImGui::SliderFloat("posZ", &posZ, -10.0f, 10.0f);
		
		}
		else if(radioBtn == 0 || radioBtn == 2){
			//GUI修改平行光方向
			ImGui::SliderFloat("dirX", &dirX, -1.0f, 1.0f);
			ImGui::SliderFloat("dirY", &dirY, -1.0f, 1.0f);
			ImGui::SliderFloat("dirZ", &dirZ, -1.0f, 1.0f);
			
		}
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		//根据不同模式，进行相对应的渲染工作
		if (radioBtn == 0 || radioBtn == 2) {
			
			//参数设置与物体渲染
			if (radioBtn == 0) {
				setShaderParameters(phoneShader, camera, true);
				drawByPhone(camera);
	
			}
			else {
				setShaderParameters(gouraudShader, camera, true);
				drawByGouraud(camera);
			}

					
			
		}
		else if (radioBtn == 1 || radioBtn == 3){
			//参数设置与物体渲染
			if (radioBtn == 1) {
				setShaderParameters(phoneShader,camera, false);
				drawByPhone(camera);
			}
			else {
				setShaderParameters(gouraudShader, camera, false);
				drawByGouraud(camera);
			}
		}
		else {
			setShaderParameters(phoneShader, camera, false);
			//点光源沿半径为5的圆上移动
			float radius = 5.0f;
			posZ = radius * cos(glfwGetTime());
			posX = radius * sin(glfwGetTime());
			posY = 0;
			drawByPhone(camera);
		}
	}

	//Gouraud Shading渲染
	void drawByGouraud(Camera &camera) {
		gouraudShader.use();
		//渲染立方体
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
		gouraudShader.setMat4("model", model);
		renderCube();
		
		//渲染球体
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0, 0.0, 0.0));
		gouraudShader.setMat4("model", model);
		renderSphere();
	}


	void drawByPhone(Camera &camera) {
		phoneShader.use();
		//渲染立方体
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
		phoneShader.setMat4("model", model);
		renderCube();

		//渲染球体
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0, 0.0, 0.0));
		phoneShader.setMat4("model", model);
		renderSphere();
	}

	//着色器参数设置
	void setShaderParameters(Shader &shader,Camera &camera,bool flag) {
		shader.use();

		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		//物体颜色，摄像机位置，物体反光度参数设置
		shader.setVec3("objectColor", glm::vec3(0.0f, 0.5f, 0.2f));
		shader.setVec3("viewPos", camera.Position);
		shader.setFloat("shininess", shininess);
		shader.setFloat("pointLight.constant", 1.0f);
		shader.setFloat("pointLight.linear", 0.09f);
		shader.setFloat("pointLight.quadratic", 0.032f);
		//平行光渲染参数设置
		if (flag) {
			lightDirection = glm::vec3(dirX, dirY, dirZ);
			shader.setVec3("dirLight.direction", lightDirection);
			shader.setVec3("dirLight.ambient", glm::vec3(ambient, ambient, ambient));
			shader.setVec3("dirLight.diffuse", glm::vec3(diffuse, diffuse, diffuse));
			shader.setVec3("dirLight.specular", glm::vec3(specular, specular, specular));

			shader.setVec3("pointLight.ambient", glm::vec3(0.0, 0.0, 0.0));
			shader.setVec3("pointLight.diffuse", glm::vec3(0.0, 0.0, 0.0));
			shader.setVec3("pointLight.specular", glm::vec3(0.0, 0.0, 0.0));
			shader.setVec3("dirLight.lightColor", glm::vec3(1.0, 1.0, 1.0));

			
		}//点光源渲染参数设置
		else {
			lightPos = glm::vec3(posX, posY, posZ);
			shader.setVec3("pointLight.lightPos", lightPos);
			shader.setVec3("pointLight.ambient", glm::vec3(ambient, ambient, ambient));
			shader.setVec3("pointLight.diffuse", glm::vec3(diffuse, diffuse, diffuse));
			shader.setVec3("pointLight.specular", glm::vec3(specular, specular, specular));

			shader.setVec3("dirLight.ambient", glm::vec3(0.0, 0.0, 0.0));
			shader.setVec3("dirLight.diffuse", glm::vec3(0.0, 0.0, 0.0));
			shader.setVec3("dirLight.specular", glm::vec3(0.0, 0.0, 0.0));
			shader.setVec3("pointLight.lightColor", glm::vec3(1.0, 1.0, 1.0));

			
		}
	}

	//渲染立方体
	void renderCube() {
		if (VAO == 0) {
			
			float cube[] = {
				// positions          // normals           // texture coords
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
				0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
				0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
				0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
				0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
				-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

				0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
				0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
				0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
				0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
				0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
				0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
				0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
				0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
			};
			

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			// fill buffer
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(VBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}


		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	//渲染球体
	void renderSphere()
	{
		if (sphereVAO == 0)
		{
			glGenVertexArrays(1, &sphereVAO);

			unsigned int sphereVBO, EBO;
			glGenBuffers(1, &sphereVBO);
			glGenBuffers(1, &EBO);

			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<unsigned int> indices;

			const unsigned int X_SEGMENTS = 64;
			const unsigned int Y_SEGMENTS = 64;
			const float PI = 3.14159265359;
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					float xSegment = (float)x / (float)X_SEGMENTS;
					float ySegment = (float)y / (float)Y_SEGMENTS;
					float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
					float yPos = std::cos(ySegment * PI);
					float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

					positions.push_back(glm::vec3(xPos, yPos, zPos));

					normals.push_back(glm::vec3(xPos, yPos, zPos));
				}
			}

			bool oddRow = false;
			for (int y = 0; y < Y_SEGMENTS; ++y)
			{
				if (!oddRow) // even rows: y == 0, y == 2; and so on
				{
					for (int x = 0; x <= X_SEGMENTS; ++x)
					{
						indices.push_back(y       * (X_SEGMENTS + 1) + x);
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					}
				}
				else
				{
					for (int x = X_SEGMENTS; x >= 0; --x)
					{
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
						indices.push_back(y       * (X_SEGMENTS + 1) + x);
					}
				}
				oddRow = !oddRow;
			}
			indexCount = indices.size();

			std::vector<float> data;
			for (int i = 0; i < positions.size(); ++i)
			{
				data.push_back(positions[i].x);
				data.push_back(positions[i].y);
				data.push_back(positions[i].z);
			
				if (normals.size() > 0)
				{
					data.push_back(normals[i].x);
					data.push_back(normals[i].y);
					data.push_back(normals[i].z);
				}
			}
			glBindVertexArray(sphereVAO);
			glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			float stride = (3 + 3) * sizeof(float);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		}

		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
	}

};