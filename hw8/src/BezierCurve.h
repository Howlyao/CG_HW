#pragma once
#ifndef  BEZIER_CURVE_H
#define BEZIER_CURVE_H

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <imgui.h>
#include <sources/imgui_impl_glfw.h>
#include <sources/imgui_impl_opengl3.h>
#include <Model/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#define MAX_CONTROL_POINT_NUM 500
#define MAX_POINT_NUM 2000
class BezierCurve {
private:
	const unsigned int SCR_WIDTH = 1280;
	const unsigned int SCR_HEIGHT = 720;

	
	Shader shader = Shader("shaders/hw8/curve.vs", "shaders/hw8/curve.fs");

	vector<glm::vec2> controlPoints;
	vector<float> controlPointCords;
	vector<float> curvePointCords;

	unsigned int controlVAO, controlVBO;
	unsigned int curveVAO, curveVBO;
	unsigned int vao, vbo;
	

	vector<glm::vec3> color;
	float t = 0.00f;
	float dt = 0.001;
	int time = 0;
	int timeout = 5;




public:
	
	BezierCurve() {
		controlPoints.clear();
		controlPointCords.clear();
		curvePointCords.clear();
		init();
	}

	void init() {
		//ControlPoint
		glGenVertexArrays(1, &controlVAO);
		glGenBuffers(1, &controlVBO);
		glBindVertexArray(controlVAO);
		glBindBuffer(GL_ARRAY_BUFFER, controlVBO);
		glBufferData(GL_ARRAY_BUFFER, MAX_CONTROL_POINT_NUM * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		//CurvePoint
		glGenVertexArrays(1, &curveVAO);
		glGenBuffers(1, &curveVBO);
		glBindVertexArray(curveVAO);
		glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINT_NUM * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		//line Point
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINT_NUM * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		//color
		for (int i = 0; i < MAX_CONTROL_POINT_NUM; i++) {
			float r = float(rand()) / RAND_MAX;
			float g = float(rand()) / RAND_MAX;
			float b = float(rand()) / RAND_MAX;
			color.push_back(glm::vec3(r, g, b));
		}
	}


	void addControlPoint(float xPos, float yPos) {
		controlPoints.push_back(glm::vec2(xPos, yPos));
		controlPointCords.push_back(xPos);
		controlPointCords.push_back(yPos);

		calculateCurvePoints();

	}

	void removeControlPoint() {
		if (!controlPoints.empty()) {
			controlPoints.pop_back();
			controlPointCords.pop_back();
			controlPointCords.pop_back();
			calculateCurvePoints();
		}
		
	}

	void calculateCurvePoints() {
		curvePointCords.clear();
		float t = 0.00f;
		int n = controlPoints.size();
		if (n == 0) return;

		while (t <= 1.0f) {
			glm::vec2 curvePoint = glm::vec2(0.0f, 0.0f);
			for (int i = 0; i < n; i++) {
				curvePoint = curvePoint + controlPoints[i] * Bernstein(i, n - 1, t);
			}
			curvePointCords.push_back(curvePoint.x);
			curvePointCords.push_back(curvePoint.y);

			t += dt;
		}

		glBindVertexArray(controlVAO);
		glBindBuffer(GL_ARRAY_BUFFER, controlVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, controlPointCords.size() * sizeof(float), &controlPointCords[0]);
		glBindVertexArray(0);

		glBindVertexArray(curveVAO);
		glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, curvePointCords.size() * sizeof(float), &curvePointCords[0]);
		glBindVertexArray(0);
		
	}

	void draw() {
		shader.use();
		drawControlPoints();
		drawCurvePoints();

		drawDynamicCurveGeneration(t);
		if (time == timeout) {
			t += dt;
			time = 0;

		}
		
		if (t >= 1.0f)
			t = 0.00f;
		time++;


	}


	void drawControlPoints() {
		if (controlPoints.empty())
			return;
		//draw control points
		shader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
		glEnable(GL_POINT_SIZE);
		glBindVertexArray(controlVAO);
		glPointSize(5);
		glDrawArrays(GL_POINTS, 0, controlPoints.size());

		//draw control points line
		shader.setVec3("color", glm::vec3(0.5f, 0.5f, 0.5f));
		glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());
		glBindVertexArray(0);
		glDisable(GL_POINT_SIZE);
	}

	//draw curve points
	void drawCurvePoints() {
		if (controlPoints.size() <= 1)
			return;
		// draw curve points
		shader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
		glPointSize(1);
		glBindVertexArray(curveVAO);
		glDrawArrays(GL_POINTS, 0, curvePointCords.size() / 2);
		glBindVertexArray(0);
	}

	//draw dynamic curve generation
	void drawDynamicCurveGeneration(float t) {
		int n = controlPoints.size();
		//当控制点的数量小于等于1时，没有动态效果
		if (n <= 1) 
			return;

		vector<float> pointCords;
		vector<glm::vec2> points;
		points.insert(points.end(), controlPoints.begin(), controlPoints.end());

		//只有控制顶数大于2时，才有线段
		for (int j = n; j > 2; j--) {
			vector<glm::vec2> temp;
			temp.clear();
			//(t-1) * point0 + t * point1
			for (int i = 0; i < j - 1; i++) {
				glm::vec2 point = (1 - t) * points[i] + t * points[i + 1];
				pointCords.push_back(point.x);
				pointCords.push_back(point.y);
				//下一层顶点
				temp.push_back(point);

			}
			
			shader.setVec3("color", color[j - 1]);
			//draw Point
			glPointSize(3);
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, pointCords.size() * sizeof(float), &pointCords[0]);
			glDrawArrays(GL_POINTS, 0, temp.size());

			//draw Line
			glDrawArrays(GL_LINE_STRIP, 0, temp.size());
			glBindVertexArray(0);

			//clear and assign temp to points
			pointCords.clear();
			points.clear();
			points.insert(points.end(), temp.begin(), temp.end());
			
			
		}

		//draw curve point when t
		shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
		int i = t / dt;
		glBindVertexArray(curveVAO);
		glPointSize(5);
		glDrawArrays(GL_POINTS, i, 1);
		glBindVertexArray(0);
		
	}

	
	//Bernstein基函数
	float Bernstein(int i, int n, float t) {
		float res;

		res = (float)frac(n) / (frac(i) * frac(n - i)) * pow(t, i) * pow(1 - t, n - i);
		return res;
	}

	//阶乘
	int frac(int n) {
		int res = 1;
		for (int i = n; i > 0; i--) {
			res *= i;
		}
		return res;
		
	}


};



#endif // ! BEZIER_CURVE_H
