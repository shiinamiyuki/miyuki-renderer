#include <ui/mykgui.h>

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
namespace Miyuki {
	namespace GUI {

		struct Item {
			Bound2i box;
			int id;
			Item(const Bound2i& box, int id) :box(box), id(id) {}
		};
		enum MouseButtonState {
			EInvalidMouseState,
			EMousePress,
			EMouseRelease
		};
		struct MouseState {
			Point2i pos;
			MouseButtonState leftButton;
			MouseButtonState rightButton;
		};

		struct MykGUI {
			Point2i windowSize;
			std::vector<Item> items;
			std::vector<Item> stack;
			MouseState mouse;
			GLFWwindow* window;
			int activeID = 0;
			int hoveredID = 0;
			int idCounter;
			static MykGUI state;
			static MykGUI& GetInstance() {
				return state;
			}
			void addItem(const Bound2i& bound, int id) {
				items.emplace_back(bound, id);
			}
			static int NextID() {
				return GetInstance().idCounter++;
			}
			static void DrawRectangle(Point2i from,
				Point2i to,
				Float depth,
				const Spectrum& color,
				bool fill) {

				int h = GetInstance().windowSize.y();
				from.y() = h - from.y();
				to.y() = h - to.y();

				auto draw = [](int x0, int y0, int x1, int y1, float depth)
				{

					glBegin(GL_QUADS);


					glVertex3f(x0, y1, depth);
					glVertex3f(x1, y1, depth);
					glVertex3f(x1, y0, depth);
					glVertex3f(x0, y0, depth);

					glEnd();
				};
				if (fill) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glColor3f(color.r(), color.g(), color.b());
					draw(from.x() - 1, from.y(), to.x(), to.y() + 1, depth);
				}
				else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor4f(color.r(), color.g(), color.b(), color.w());
					draw(from.x(), from.y(), to.x(), to.y(), depth);
				}
			}

			static void DrawRectangle(const Bound2i& box, Float depth, const Spectrum& color,
				bool fill) {
				DrawRectangle(box.pMin, box.pMax, depth, color, fill);
			}

			static bool MouseIn(Bound2i& box) {
				auto& instance = MykGUI::GetInstance();
				return box.contains(instance.mouse.pos);
			}

			static MouseButtonState FromGLFWMouseState(int s) {
				if (s == GLFW_PRESS)
					return EMousePress;
				if (s == GLFW_RELEASE)
					return EMouseRelease;
				return EInvalidMouseState;
			}
			static void BeginFrame(int w, int h) {
				auto& instance = MykGUI::GetInstance();
				instance.idCounter = 0;
				instance.items.clear();
				instance.stack.clear();
				instance.windowSize.x() = w;
				instance.windowSize.y() = h;
				double xpos, ypos;
				glfwGetCursorPos(instance.window, &xpos, &ypos);
				instance.mouse.pos.x() = xpos;
				instance.mouse.pos.y() = ypos;
				instance.mouse.leftButton =
					FromGLFWMouseState(glfwGetMouseButton(instance.window, GLFW_MOUSE_BUTTON_LEFT));
				instance.mouse.rightButton =
					FromGLFWMouseState(glfwGetMouseButton(instance.window, GLFW_MOUSE_BUTTON_RIGHT));
				glPushMatrix();
				glLoadIdentity();
				glOrtho(0, instance.windowSize.x(), 0, instance.windowSize.y(), -1, 1);
			}

			static void EndFrame() {
				glPopMatrix();
			}
			
			// Create a MykGUI window
			static bool Begin(const char* title,
				bool * open) {
				
			}

			static bool BeginHorizontalLayout() {

			}

			static bool Button() {
				Bound2i box({ 100,100 }, { 300,300 });
				auto& instance = GetInstance();
				auto id = NextID();
				bool result = false;
				bool inside = MouseIn(box);
				if (inside) {
					if (instance.mouse.leftButton == EMousePress)
						instance.activeID = id;
					instance.hoveredID = id;
				}
				if (instance.mouse.leftButton == EMouseRelease) {
					if (state.activeID == id) {
						state.activeID = -1;
						if (inside)
							result = true;
					}
				}
				if (instance.activeID == id) {
					MykGUI::DrawRectangle(box, 0, { 1,0,0 }, true);
				}
				else {
					MykGUI::DrawRectangle(box, 0, { 1,1,1 }, true);
				}
				return result;
			}
			static void CreateContext(GLFWwindow* window) {
				MykGUI::GetInstance().window = window;
			}
		};

		MykGUI MykGUI::state;		
		


		Window::Window(const char* title) :_title(title) {
			glfwSetErrorCallback(glfw_error_callback);
			if (!glfwInit())
				std::exit(1);
			window = glfwCreateWindow(1920, 1080, _title.c_str(), NULL, NULL);
			if (window == NULL)
				std::exit(1);
			glfwMakeContextCurrent(window);
			glfwSwapInterval(1);
			MykGUI::CreateContext(window);
		}
		void Window::update() {

		}
		void Window::show() {
			while (!glfwWindowShouldClose(window)) {
				glfwPollEvents();

				update();
				int display_w, display_h;
				glfwMakeContextCurrent(window);
				glfwGetFramebufferSize(window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
			
				glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), 255);
				glClear(GL_COLOR_BUFFER_BIT);

				MykGUI::BeginFrame(display_w, display_h);

				MykGUI::Button();

				MykGUI::EndFrame();
				glfwMakeContextCurrent(window);
				glfwSwapBuffers(window);

				//std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
			}
		}
	}
}