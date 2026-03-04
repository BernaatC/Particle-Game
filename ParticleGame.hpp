// Standard libraries used in this code
#include <math.h>
#include <list>
#include <vector>
#include <string>
#include <unordered_map>
#include <ctime>

//#include <iostream>

// External libraries used for various purposes
#include "pugixml/pugixml.hpp" // Read and write through XML
#include <raylib.h> // Drawing graphics and user input


// Title of the game and version, moved here for convenience
static const std::string GAME_TITLE = "Particle Game";
static const std::string GAME_VERSION = "Ver. 1.0";


// Other fixed text messages given to the player
static const std::string STAGE_SUCCESS_MESSAGE = "Good job!";
static const std::string STAGE_FAILURE_MESSAGE = "Try again!";

static const std::vector<std::string> CONTROLS_TEXT = {
	"Controls:",
	"- O: Change particle polarity",
	"- P: Start/End simulation",
	"- Q: Return to main menu",
	"- R: Store save data",
	"- Drag and drop to load save data"
};

static const std::string PALETTE_SUCCESSFUL_LOAD = "Palette loaded successfuly";

// Used for changing between floats and doubles program-wide
#define DOUBLE

#ifdef FLOAT
	#define Float float
	#define SIN(X) sinf(X)
	#define COS(X) cosf(X)
	#define ACOS(X) acosf(X)
	#define ATAN2(Y,X) atan2f(Y,X)
	#define AS_FLOAT(X) X.as_float()
	#define ROUND(X) std::roundf(X)
#else
	#define Float double
	#define SIN(X) sin(X)
	#define COS(X) cos(X)
	#define ACOS(X) acos(X)
	#define ATAN2(Y,X) atan2(Y,X)
	#define AS_FLOAT(X) X.as_double()
	#define ROUND(X) std::round(X)
#endif


// Substitutions used for ease of coding
#define id_val unsigned int
#define C_PI 3.1415926535897932384626


// Variables used for drawing graphics / detecting mouse input
static const int VIRTUAL_SCREEN_WIDTH         = 800;
static const int VIRTUAL_SCREEN_HEIGHT        = 600;

static const Float REAL_VIRTUAL_RATIO         = 1.5;

static const int REAL_SCREEN_WIDTH            = VIRTUAL_SCREEN_WIDTH * REAL_VIRTUAL_RATIO;
static const int REAL_SCREEN_HEIGHT           = VIRTUAL_SCREEN_HEIGHT * REAL_VIRTUAL_RATIO;

static const int FPS                          = 60;
static const int GAME_UPDATES_PER_FRAME       = 16;

static const Float PARTICLE_RADIUS            = 10;
static const Float PARTICLE_LINE_THICKNESS    = 2;

static const int TEXT_FONT_SIZE_TITLE         = 50;
static const int TEXT_FONT_SIZE_SUBTITLE	  = 30;
static const int TEXT_FONT_SIZE_NORMAL        = 20;

// Variables used for accessing files
static const std::string OFFICIAL_STAGES_DIR("./stages/official");
static const std::string CUSTOM_STAGES_DIR("./stages/custom");
static const std::string TUTORIALS_DIR("./stages/tutorials");
static const std::string SAVES_DIR("./saves");
static const std::string DEFAULT_PALETTE_DIR("./palettes/default.xml");

static const std::string XML_FILE_EXTENSION(".xml");



class Vec2D {
	public:
		typedef enum Vec2DInitType {
			CARTESIAN = 0,
			POLAR
		};

		Vec2D() : x(0), y(0) {}

		Vec2D(const Float& x_r, const Float& y_th, const Vec2DInitType& initType = CARTESIAN) {
			switch (initType) {
				case CARTESIAN:
					x = x_r;
					y = y_th;
					break;

				case POLAR:
					x = x_r * COS(y_th);
					y = x_r * SIN(y_th);
					break;

				default:
					break;
			}
		}

		~Vec2D() {}


		void set(const Float& p_x, const Float& p_y) {
			x = p_x;
			y = p_y;
		}

		Vec2D operator+(const Vec2D& c) const {
			Vec2D returnCoord(x + c.x, y + c.y, CARTESIAN);
			return returnCoord;
		}

		Vec2D operator-(const Vec2D& c) const {
			Vec2D returnCoord(x - c.x, y - c.y, CARTESIAN);
			return returnCoord;
		}

		Float operator*(const Vec2D& c) const {
			// Cross product
			Float returnVal = (x * c.y) - (c.x * y);
			return returnVal;
		}

		Float elementSum() const {
			return x + y;
		}

		Float squaredElementSum() const {
			return (x * x) + (y * y);
		}


		Vec2D operator*(const Float& scale) const {
			Vec2D returnCoord(x * scale, y * scale, CARTESIAN);
			return returnCoord;
		}

		Vec2D operator/(const Float& scale) const {
			Vec2D returnCoord(x / scale, y / scale, CARTESIAN);
			return returnCoord;
		}


		Vec2D operator=(const Vec2D& c) {
			x = c.x;
			y = c.y;
			return *this;
		}


		bool operator==(const Vec2D& c) {
			return ((x == c.x) && (y == c.y));
		}


		Float x, y;
};

class VecUtils {
	public:
		static Float distance(const Vec2D& c1, const Vec2D& c2) {
			Vec2D elementwiseDist = c1 - c2;
			Float squaredDist = elementwiseDist.squaredElementSum();
			return sqrtf(squaredDist);
		}

		static Float squaredDistance(const Vec2D& c1, const Vec2D& c2) {
			Vec2D elementwiseDist = c1 - c2;
			return elementwiseDist.squaredElementSum();
		}

		static Float getAngle(const Vec2D& c1, const Vec2D& c2) {
			return ATAN2(c1.y - c2.y, c1.x - c2.x);
		}

		static Float angle3(const Vec2D& c0, const Vec2D& c1, const Vec2D& c2) {
			// Uses law of cosines to get angle between two lines defined
			// by two points each (c0 is shared between both lines)

			Float dist01 = distance(c0, c1);
			Float dist02 = distance(c0, c2);
			Float dist12 = distance(c1, c2);
			Float numerator = (dist01 * dist01) + (dist02 * dist02) - (dist12 * dist12);
			Float denominator = 2 * dist01 * dist02;

			Float angle = ACOS(numerator / denominator);
			if (angle > C_PI) {
				angle = (2 * C_PI) - angle;
			}

			return angle;
		}

		static Vec2D rotatePoint(const Vec2D& point, const Vec2D& center, const Float& rotateAngle) {
			Vec2D translatedPoint = point - center;
			Float dist = distance(point, center);
			Float angle = ATAN2(translatedPoint.y, translatedPoint.x);

			Vec2D newPointTranslated(dist, angle + rotateAngle, Vec2D::POLAR);
			return newPointTranslated + center;
		}

		static Vec2D stringToVec(const std::string& str) {
			Vec2D returnVec;
			int commaPosition = str.find(',');
			returnVec.x = std::stoi(str.substr(0, commaPosition));
			returnVec.y = std::stoi(str.substr(commaPosition + 1, str.length()));
			return returnVec;
		}

		static Vec2D decipherStringToVec(const std::string& str, const std::unordered_map<unsigned int, Vec2D>& points) {
			if (str.find(',') != std::string::npos) {
				return stringToVec(str);
			}

			else {
				return points.at((unsigned int)std::stoi(str));
			}
		}

		static std::vector<Vec2D> decipherStringToVecArray(const std::string& str, const std::unordered_map<unsigned int, Vec2D>& points) {
			std::vector<std::string> pointsStr;
			size_t previousPos = 0, currentPos = str.find(';', previousPos);

			while (currentPos != std::string::npos) {
				pointsStr.push_back(str.substr(previousPos, currentPos));
				previousPos = currentPos + 1;
				currentPos = str.find(';', previousPos);
			}
			pointsStr.push_back(str.substr(previousPos, str.length()));

			std::vector<Vec2D> returnArray;

			for (int i = 0; i < pointsStr.size(); i++) {
				returnArray.push_back(decipherStringToVec(pointsStr[i], points));
			}

			return returnArray;
		}

		static Float triangleArea(const Vec2D& c1, const Vec2D& c2, const Vec2D& c3) {
			return abs((c2 - c1) * (c3 - c1)) / 2;
		}
};


class DrawOptions {
	public:
		DrawOptions() : m_translation(), m_scale(REAL_VIRTUAL_RATIO) {}
		
		DrawOptions(const Vec2D& p_translation, const Float& p_scale) : m_translation(p_translation), m_scale(p_scale) {}

		DrawOptions(const DrawOptions& d1, const DrawOptions& d2) {  // Merges two different draw option objects into one
			m_translation = d1.m_translation + d2.m_translation;
			m_scale = d1.m_scale * d2.m_scale;
		}

		~DrawOptions() {}


		Vec2D transform(const Vec2D& c) const {
			Vec2D returnVec = (c + m_translation) * m_scale;
			return returnVec;
		}

		Vec2D inverseScale(const Vec2D& c) const {
			Vec2D returnVec = (c + m_translation) / m_scale;
			return returnVec;
		}

		Vec2D scaleOnly(const Vec2D& c) const {
			Vec2D returnVec = c * m_scale;
			return returnVec;
		}

		Float scaleOnly(const Float& x) const {
			return x * m_scale;
		}

		int scaleOnly(const int& i) const {
			return i * m_scale;
		}

	private:
		Vec2D m_translation;
		Float m_scale;
};

class Colour {
	public:
		Colour() : r(0), g(0), b(0), a(255) {}
			
		Colour(const unsigned char& p_r, const unsigned char& p_g, const unsigned char& p_b, const unsigned char& p_a = 255) : r(p_r), g(p_g), b(p_b), a(p_a) {}

		Colour(const Colour& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}

		Colour(const unsigned int& vals) {
			r = (vals >> 24) & 255;
			g = (vals >> 16) & 255;
			b = (vals >>  8) & 255;
			a =  vals        & 255;
		}

		~Colour() {}


		Colour& operator=(const Colour& c) {
			r = c.r;
			g = c.g;
			b = c.b;
			a = c.a;
			return *this;
		}

		Colour& operator=(const unsigned int& vals) {
			r = (vals >> 24) & 255;
			g = (vals >> 16) & 255;
			b = (vals >>  8) & 255;
			a =  vals        & 255;
			return *this;
		}


		unsigned char r, g, b, a;
};

class ColourPalette {
	public:
		ColourPalette(const std::string& filePath = DEFAULT_PALETTE_DIR) {
			updateColourPalette(filePath, colours);
		}


		~ColourPalette() {}


		void update(const std::string& filePath) {
			updateColourPalette(filePath, colours);
		}


		std::unordered_map<std::string, Colour> colours;

	private:
		void updateColourPalette(const std::string& filePath, std::unordered_map<std::string, Colour>& colours) {
			pugi::xml_document file;
			pugi::xml_parse_result stageXML = file.load_file(filePath.c_str());

			pugi::xml_node rootNode = file.child("palette");

			for (auto it = rootNode.begin(); it != rootNode.end(); it++) {
				std::string valString = it->attribute("val").as_string();
				Colour colour = std::stoul(valString.substr(2), 0, 16);

				colours[it->name()] = colour;
			}
		}
};
	

class Draw {
	public:
		static Color colourToRaylibColor(const Colour& c) {
			Color returnVal;
			returnVal.r = c.r;
			returnVal.g = c.g;
			returnVal.b = c.b;
			returnVal.a = c.a;
			return returnVal;
		}

		static Vector2 coord2DToRaylibVector2(const Vec2D& c) {
			Vector2 returnVector;
			returnVector.x = c.x;
			returnVector.y = c.y;
			return returnVector;
		}


		static void background(const Colour& colour) {
			ClearBackground(colourToRaylibColor(colour));
		}

		static void line(const Colour colour, const Vec2D& point1, const Vec2D& point2) {
			Color raylibColour = colourToRaylibColor(colour);

			DrawLine(point1.x, point1.y, point2.x, point2.y, raylibColour);
		}

		static void circle(const Colour& fillColour, const Vec2D& center, const Float& radius) {
			Color raylibFillColour = colourToRaylibColor(fillColour);

			DrawCircle(center.x, center.y, radius, raylibFillColour);
		}

		static void bagel(const Colour& fillColour, const Vec2D& center, const Float& innerRadius, const Float& outerRadius) {
			Color raylibFillColour = colourToRaylibColor(fillColour);
			Vector2 raylibPosition = coord2DToRaylibVector2(center);

			DrawRing(raylibPosition, innerRadius, outerRadius, 0, 360, 360, raylibFillColour);
		}

		static void triangle(const Colour& fillColour, const Vec2D& point1, const Vec2D& point2, const Vec2D& point3) {
			Vector2 raylibPoint1 = coord2DToRaylibVector2(point1);
			Vector2 raylibPoint2 = coord2DToRaylibVector2(point2);
			Vector2 raylibPoint3 = coord2DToRaylibVector2(point3);

			Color raylibFillColour = colourToRaylibColor(fillColour);

			DrawTriangle(raylibPoint1, raylibPoint2, raylibPoint3, raylibFillColour);
		}

		static void rectangle(const Colour& buttonColour, const Vec2D& upperLeftCorner, const Vec2D& size) {
			Color raylibColour = colourToRaylibColor(buttonColour);

			DrawRectangle(upperLeftCorner.x, upperLeftCorner.y, size.x, size.y, raylibColour);
		}

		static void regularPolygon(const Colour& fillColour, const size_t nSides, const Vec2D& center, const Float& radius, const Float& angle) {
			Vector2 raylibCenter = coord2DToRaylibVector2(center);

			Color raylibFillColour = colourToRaylibColor(fillColour);

			DrawPoly(raylibCenter, nSides, radius, angle, raylibFillColour);
		}

		static void text(const Colour& colour, const Vec2D& position, const int& fontSize, const std::string& text) {
			Color raylibColour = colourToRaylibColor(colour);

			DrawText(text.c_str(), position.x, position.y, fontSize, raylibColour);
		}
};

class Input {
	public:
		static Vec2D getMousePosition(const DrawOptions& drawOptions) {
			Vec2D mousePosition(GetMouseX(), GetMouseY(), Vec2D::CARTESIAN);
			mousePosition = drawOptions.inverseScale(mousePosition);

			return mousePosition;
		}

		static bool isKeyReleased(const char& key) {
			return IsKeyReleased(charToKeyCode(key));
		}

		static bool isMouseReleased(const char& button) {
			return IsMouseButtonReleased(charToMouseButton(button));
		}


		static std::vector<std::string> loadFileNames(const std::string directory, const size_t start, const size_t end) {
			FilePathList raylibFileList = LoadDirectoryFiles(directory.c_str());
			std::vector<std::string> fileList;

			for (size_t i = start; i < end; i++) {
				if (i < raylibFileList.count) {
					std::string fileName = raylibFileList.paths[i];
					fileName = fileName.substr(directory.size() + 1);
					fileList.push_back(fileName);
				}

				else {
					fileList.push_back("");
				}
			}

			UnloadDirectoryFiles(raylibFileList);

			return fileList;
		}

		static size_t getItemsInDirectory(const std::string directory) {
			FilePathList raylibFileList = LoadDirectoryFiles(directory.c_str());
			size_t nFiles = raylibFileList.count;
			UnloadDirectoryFiles(raylibFileList);
			return nFiles;
		}


		static bool itemsBeenDropped() {
			return IsFileDropped();
		}

		static std::string getDroppedFileLocation() {
			FilePathList droppedFiles = LoadDroppedFiles();
			std::string returnString = droppedFiles.paths[0];
			UnloadDroppedFiles(droppedFiles);
			return returnString;
		}

	private:
		static KeyboardKey charToKeyCode(const char& c) {
			switch (c) {
				case 'A': return KEY_A;
				case 'B': return KEY_B;
				case 'C': return KEY_C;
				case 'D': return KEY_D;
				case 'E': return KEY_E;
				case 'F': return KEY_F;
				case 'G': return KEY_G;
				case 'H': return KEY_H;
				case 'I': return KEY_I;
				case 'J': return KEY_J;
				case 'K': return KEY_K;
				case 'L': return KEY_L;
				case 'M': return KEY_M;
				case 'N': return KEY_N;
				case 'O': return KEY_O;
				case 'P': return KEY_P;
				case 'Q': return KEY_Q;
				case 'R': return KEY_R;
				case 'S': return KEY_S;
				case 'T': return KEY_T;
				case 'U': return KEY_U;
				case 'V': return KEY_V;
				case 'W': return KEY_W;
				case 'X': return KEY_X;
				case 'Y': return KEY_Y;
				case 'Z': return KEY_Z;
				default: return KEY_NULL;
			}
		}

		static MouseButton charToMouseButton(const char& c) {
			switch (c) {
				case 'L': return MOUSE_BUTTON_LEFT;
				case 'R': return MOUSE_BUTTON_RIGHT;
				case 'C': return MOUSE_BUTTON_MIDDLE;
				default: return MOUSE_BUTTON_EXTRA;
			}
		}
};


class Animation {
	public:
		typedef struct Target {
			typedef enum Type {
				GOAL = 0,
				OBSTACLE,
				MAGNETIC_FIELD
			};

			Type tType;
			unsigned int index;
		};

		typedef struct MovementParams {
			Vec2D translation;
			Float rotation;
			Float scale;
			bool toggle;
		};

		typedef struct Movement {
			Target target;
			MovementParams move;
			bool animationApplied;
		};

		Animation(const std::vector<Float> speedParams, const unsigned long& start, const unsigned long& end,
				  const unsigned long& loop, const Target& target) {

			m_speedParams = speedParams;
			m_start = start;
			m_end = end;
			m_loop = loop;
			m_target.tType = target.tType;
			m_target.index = target.index;
			m_doesLoop = !(loop == 0);
		}

		~Animation() {}

		virtual Movement calculateAnim(const unsigned long& updateCounter) const = 0;

		Float getSpeedVal(const Float& t) const {
			Float ti = t;
			Float speedVal = 0;

			for (int i = 0; i < m_speedParams.size(); i++) {
				speedVal += m_speedParams[i] * ti;
				ti *= t;
			}

			return speedVal;
		}

		static Target::Type stringToTargetType(const std::string& str) {

			if (str == "goal") {
				return Target::GOAL;
			}

			else if (str == "obstacle") {
				return Target::OBSTACLE;
			}

			return Target::MAGNETIC_FIELD;
		}


	protected:
		std::vector<Float> m_speedParams;
		unsigned long m_start, m_end, m_loop;
		bool m_doesLoop;
		Target m_target;
};

class LinealTranslate : public Animation {
	public:
		LinealTranslate(const Vec2D& endPoint, const std::vector<Float> speedParams, const unsigned long& start,
						const unsigned long& end, const unsigned long& loop, const Target& target) :
			Animation(speedParams, start, end, loop, target) {

			m_endPoint = endPoint;
		}

		~LinealTranslate() {}


		Movement calculateAnim(const unsigned long& updateCounter) const {
			Movement appliedMovement;

			appliedMovement.target = m_target;
			appliedMovement.move.rotation = 0;
			appliedMovement.move.toggle = false;
			appliedMovement.move.scale = 1;

			unsigned long realCounter = m_doesLoop ? updateCounter % m_loop : updateCounter;

			appliedMovement.animationApplied = (realCounter >= m_start && realCounter < m_end);
			if (appliedMovement.animationApplied) {
				Float currentAnimChunk = (Float)(realCounter - m_start + 1) / (m_end - m_start);
				Float nextAnimChunk = currentAnimChunk + (1.0 / (m_end - m_start));

				Float currentPositionVal = getSpeedVal(currentAnimChunk);
				Float nextPositionVal = getSpeedVal(nextAnimChunk);

				Float diff = nextPositionVal - currentPositionVal;

				appliedMovement.move.translation = m_endPoint * diff;
			}

			return appliedMovement;
		}

	private:
		Vec2D m_endPoint;
};

class CircleTranslate : public Animation {
	public:
		CircleTranslate(const Float& startAngle, const Float& endAngle, const Float distanceFromRotationCenter,
						const std::vector<Float> speedParams, const unsigned long& start, const unsigned long& end,
						const unsigned long& loop, const Target& target) :
			Animation(speedParams, start, end, loop, target) {

			m_startAngle = startAngle;
			m_endAngle = endAngle;

			m_distanceFromRotationCenter = distanceFromRotationCenter;
		}

		~CircleTranslate() {}


		Movement calculateAnim(const unsigned long& updateCounter) const {
			Movement appliedMovement;

			appliedMovement.target = m_target;
			appliedMovement.move.rotation = 0;
			appliedMovement.move.toggle = false;
			appliedMovement.move.scale = 1;

			unsigned long realCounter = m_doesLoop ? updateCounter % m_loop : updateCounter;

			appliedMovement.animationApplied = (realCounter >= m_start && realCounter < m_end);
			if (appliedMovement.animationApplied) {
				Float currentAnimChunk = (Float)(realCounter - m_start + 1) / (m_end - m_start);
				Float nextAnimChunk = currentAnimChunk + (1.0 / (m_end - m_start));

				Float currentPositionVal = getSpeedVal(currentAnimChunk);
				Float nextPositionVal = getSpeedVal(nextAnimChunk);

				Float currentAngleVal = m_startAngle + (currentPositionVal * (m_endAngle - m_startAngle));
				Float nextAngleVal = m_startAngle + (nextPositionVal * (m_endAngle - m_startAngle));

				Vec2D currentAnglePositionVal(m_distanceFromRotationCenter, currentAngleVal, Vec2D::POLAR);
				Vec2D nextAnglePositionVal(m_distanceFromRotationCenter, nextAngleVal, Vec2D::POLAR);

				appliedMovement.move.translation = nextAnglePositionVal - currentAnglePositionVal;
			}

			return appliedMovement;
		}

	private:
		Float m_startAngle, m_endAngle;

		Float m_distanceFromRotationCenter;
};

class BezierTranslate : public Animation {
	public:
		BezierTranslate(const std::vector<Vec2D>& controlPoints, const std::vector<Float> speedParams,
						const unsigned long& start, const unsigned long& end, const unsigned long& loop,
						const Target& target) : Animation(speedParams, start, end, loop, target) {

			m_controlPoints = controlPoints;
		}

		~BezierTranslate() {}

		Vec2D calcPoint(const Float& t) const {
			size_t n = m_controlPoints.size() - 1; // degree of the Bezier curve <- n
			Vec2D returnPoint;

			for (size_t i = 0; i <= n; i++) {
				Float mulVal = binomialCoefficient(n, i);

				mulVal *= (t == 0) ? 1 : powf(t, i);
				mulVal *= (t == 1) ? 1 : powf(1 - t, n - i);

				returnPoint = returnPoint + (m_controlPoints[i] * mulVal);
			}

			return returnPoint;
		}

		Movement calculateAnim(const unsigned long& updateCounter) const {
			Movement appliedMovement;

			appliedMovement.target = m_target;
			appliedMovement.move.rotation = 0;
			appliedMovement.move.toggle = false;
			appliedMovement.move.scale = 1;

			unsigned long realCounter = m_doesLoop ? updateCounter % m_loop : updateCounter;

			appliedMovement.animationApplied = (realCounter >= m_start && realCounter < m_end);
			if (appliedMovement.animationApplied) {
				Float currentAnimChunk = (Float)(realCounter - m_start + 1) / (m_end - m_start);
				Float nextAnimChunk = currentAnimChunk + (1.0 / (m_end - m_start));

				Float currentPositionVal = getSpeedVal(currentAnimChunk);
				Float nextPositionVal = getSpeedVal(nextAnimChunk);

				appliedMovement.move.translation = calcPoint(nextPositionVal) - calcPoint(currentPositionVal);
			}

			return appliedMovement;
		}

	private:
		std::vector<Vec2D> m_controlPoints;

		Float binomialCoefficient(const Float& n, const Float& k) const {
			return tgammaf(n + 1) / (tgammaf(k + 1) * tgamma(n - k + 1));
		}
};

class Rotate : public Animation {
	public:
		Rotate(const Float& angle, const std::vector<Float> speedParams, const unsigned long& start,
			   const unsigned long& end, const unsigned long& loop, const Target& target) :
			Animation(speedParams, start, end, loop, target) {

			m_angle = angle;
		}

		~Rotate() {}


		Movement calculateAnim(const unsigned long& updateCounter) const {
			Movement appliedMovement;

			appliedMovement.target = m_target;
			appliedMovement.move.translation.set(0, 0);
			appliedMovement.move.toggle = false;
			appliedMovement.move.scale = 1;

			unsigned long realCounter = m_doesLoop ? updateCounter % m_loop : updateCounter;

			appliedMovement.animationApplied = (realCounter >= m_start && realCounter < m_end);
			if (appliedMovement.animationApplied) {
				Float currentAnimChunk = (Float)(realCounter - m_start) / (m_end - m_start);
				Float nextAnimChunk = currentAnimChunk + (1.0 / (m_end - m_start));

				Float currentPositionVal = getSpeedVal(currentAnimChunk);
				Float nextPositionVal = getSpeedVal(nextAnimChunk);

				Float diff = nextPositionVal - currentPositionVal;

				appliedMovement.move.rotation = m_angle * diff;
			}

			return appliedMovement;
		}

	private:
		Float m_angle;
};

class Toggle : public Animation {
	public:
		Toggle(const unsigned long& time, const unsigned long& loop, const Target& target) : Animation({}, time, time, loop, target) {}

		~Toggle() {}


		Movement calculateAnim(const unsigned long& updateCounter) const {
			Movement appliedMovement;

			appliedMovement.target = m_target;
			appliedMovement.move.translation.set(0, 0);
			appliedMovement.move.rotation = 0;
			appliedMovement.move.scale = 1;

			unsigned long realCounter = m_doesLoop ? updateCounter % m_loop : updateCounter;

			appliedMovement.move.toggle = (realCounter == m_start);
			appliedMovement.animationApplied = appliedMovement.move.toggle;

			return appliedMovement;
		}

		unsigned long getTime() const {
			return m_start;
		}
};

class Scale : public Animation {
	public:
		Scale(const Float& scaleFactor, const std::vector<Float> speedParams, const unsigned long& start,
			  const unsigned long& end, const unsigned long& loop, const Target& target) :
			Animation(speedParams, start, end, loop, target) {
			
			m_scaleFactor = getRealScaleFactor(start, end, scaleFactor / 2);
		}

		~Scale() {}


		Movement calculateAnim(const unsigned long& updateCounter) const {
			Movement appliedMovement;

			appliedMovement.target = m_target;
			appliedMovement.move.translation.set(0, 0);
			appliedMovement.move.rotation = 0;
			appliedMovement.move.toggle = false;

			unsigned long realCounter = m_doesLoop ? updateCounter % m_loop : updateCounter;

			appliedMovement.animationApplied = (realCounter >= m_start && realCounter < m_end);
			if (appliedMovement.animationApplied) {
				Float currentAnimChunk = (Float)(realCounter - m_start) / (m_end - m_start);
				Float nextAnimChunk = currentAnimChunk + (1.0 / (m_end - m_start));

				Float currentPositionVal = getSpeedVal(currentAnimChunk);
				Float nextPositionVal = getSpeedVal(nextAnimChunk);

				Float currentScaleFactor = (1 + nextPositionVal) / (1 + currentPositionVal);

				appliedMovement.move.scale = currentScaleFactor * m_scaleFactor;
			}

			return appliedMovement;
		}

	private:
		Float m_scaleFactor;

		// For some reason, doing the n-th root does not yield the correct result, therefore constructor gets the value manually
		Float getRealScaleFactor(const unsigned long& start, const unsigned long& end, const Float& givenScaleFactor) {
			unsigned long dt = end - start;
			Float lowerBound = 1;
			Float upperBound = givenScaleFactor;

			Float prevLowerBound, prevUpperBound;
			Float possibleValue, result;
			do {
				prevLowerBound = lowerBound;
				prevUpperBound = upperBound;
				possibleValue = (lowerBound + upperBound) / 2;
				result = 1;
				unsigned long i = 0;
				while ((i < dt) && (result < givenScaleFactor)) {
					result *= possibleValue;
					i++;
				}
				if (result > givenScaleFactor) {
					upperBound = possibleValue;
				}
				else if (result < givenScaleFactor) {
					lowerBound = possibleValue;
				}
			} while ((result != givenScaleFactor) && ((lowerBound != prevLowerBound) || (upperBound != prevUpperBound)));

			static const Float c_step = 0.0000001;  // 1e-7

			if (result < givenScaleFactor) {
				Float prevResult, prevPossibleValue;
				do {
					prevResult = result;
					prevPossibleValue = possibleValue;
					result = 1;
					possibleValue += c_step;
					for (unsigned long i = 0; i < dt; i++) {
						result *= possibleValue;
					}
				} while (result < givenScaleFactor);

				return (givenScaleFactor - prevPossibleValue < possibleValue - givenScaleFactor) ? prevPossibleValue : possibleValue;
			}

			else if (result > givenScaleFactor) {
				Float prevResult, prevPossibleValue;
				do {
					prevResult = result;
					prevPossibleValue = possibleValue;
					result = 1;
					possibleValue -= c_step;
					for (unsigned long i = 0; i < dt; i++) {
						result *= possibleValue;
					}
				} while (result > givenScaleFactor);

				return (givenScaleFactor - possibleValue > prevPossibleValue - givenScaleFactor) ? prevPossibleValue : possibleValue;
			}

			return possibleValue;
		}
};


class Shape {
	public:
		typedef enum Function {
			NONE = 0,
			OBSTACLE,
			MAGNETIC_FIELD_IN,
			MAGNETIC_FIELD_OUT
		};

		Shape(const Shape::Function& startingShapeFunction, const Shape::Function& intendedShapeFunction) {
			m_originalShapeFunction = startingShapeFunction;
			m_intendedShapeFunction = intendedShapeFunction;
			m_shapeFunction = startingShapeFunction;
		}

		~Shape() {}


		void toggle(const bool& shouldToggle) {
			m_shapeFunction = ((m_shapeFunction == NONE) ^ shouldToggle) ? NONE : m_intendedShapeFunction;
		}

		Shape::Function getShapeFunction() const {
			return m_shapeFunction;
		}


		virtual bool isTouching(const Vec2D& particleCenter) const = 0;


		virtual void translate(const Vec2D& translation) = 0;

		virtual void rotate(const Float& angle) = 0;

		virtual void scale(const Float& scaleFactor) = 0;


		void applyAnim(const Animation::Movement& movement) {
			rotate(movement.move.rotation);
			translate(movement.move.translation);
			toggle(movement.move.toggle);
			scale(movement.move.scale);
		}


		virtual void reset() = 0;


		Colour getColour(const ColourPalette& colourPalette) const {
			static const Colour d_transparent = 0x00000000;

			switch (m_shapeFunction) {
				case OBSTACLE:
					return colourPalette.colours.at("obstacleFill");

				case MAGNETIC_FIELD_IN:
					return colourPalette.colours.at("magneticFieldInFill");

				case MAGNETIC_FIELD_OUT:
					return colourPalette.colours.at("magneticFieldOutFill");

				default: // Should only enter if m_shapeFunction == NONE
					return d_transparent;
			}
		}

		virtual void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const = 0;

	protected:
		Shape::Function m_originalShapeFunction;
		Shape::Function m_intendedShapeFunction;
		Shape::Function m_shapeFunction;
};

class ShapeCircle : public Shape {
	public:
		ShapeCircle(const Vec2D& center, const Float& radius, const Shape::Function& startingShapeFunction,
					const Shape::Function& intendedShapeFunction) : Shape(startingShapeFunction, intendedShapeFunction) {

			m_originalCenter = center;
			m_center = center;

			m_radius = radius;
			m_originalRadius = radius;
		}

		~ShapeCircle() {}


		bool isTouching(const Vec2D& particleCenter) const {
			Float dist = VecUtils::distance(particleCenter, m_center);
			return (dist < m_radius + PARTICLE_RADIUS);
		}


		void translate(const Vec2D& translation) {
			m_center = m_center + translation;
		}

		void rotate(const Float& angle) {
			// Yeah, what did you think this was gonna do genius?
		}

		void scale(const Float& scaleFactor) {
			m_radius *= scaleFactor;
		}

		void reset() {
			m_center = m_originalCenter;
			m_radius = m_originalRadius;
			m_shapeFunction = m_originalShapeFunction;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Float realRadius = drawOptions.scaleOnly(m_radius);

			Vec2D realCenter = drawOptions.transform(m_center);

			Colour fillColour = getColour(colourPalette);

			Draw::circle(fillColour, realCenter, realRadius);
		}

	private:
		Vec2D m_originalCenter;
		Vec2D m_center;

		Float m_radius;
		Float m_originalRadius;
};

class ShapeBagel : public Shape {
	public:
		ShapeBagel(const Vec2D& center, const Float& innerRadius, const Float& outerRadius, const Shape::Function& startingShapeFunction, 
				   const Shape::Function& intendedShapeFunction) : Shape(startingShapeFunction, intendedShapeFunction) {

			m_originalCenter = center;
			m_center = center;
			m_innerRadius = innerRadius;
			m_outerRadius = outerRadius;
			m_originalInnerRadius = innerRadius;
			m_originalOuterRadius = outerRadius;
		}

		~ShapeBagel() {}


		bool isTouching(const Vec2D& particleCenter) const {
			Float dist = VecUtils::distance(particleCenter, m_center);
			return (dist < m_outerRadius + PARTICLE_RADIUS && dist > m_innerRadius - PARTICLE_RADIUS);
		}


		void translate(const Vec2D& translation) {
			m_center = m_center + translation;
		}

		void rotate(const Float& angle) {
			// Refer to Circle.rotate()
		}

		void scale(const Float& scaleFactor) {
			m_innerRadius *= scaleFactor;
			m_outerRadius *= scaleFactor;
		}

		void reset() {
			m_center = m_originalCenter;
			m_innerRadius = m_originalInnerRadius;
			m_outerRadius = m_originalOuterRadius;
			m_shapeFunction = m_originalShapeFunction;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Float realInnerRadius = drawOptions.scaleOnly(m_innerRadius);
			Float realOuterRadius = drawOptions.scaleOnly(m_outerRadius);

			Vec2D realCenter = drawOptions.transform(m_center);

			Colour fillColour = getColour(colourPalette);

			Draw::bagel(fillColour, realCenter, realInnerRadius, realOuterRadius);
		}

	private:
		Vec2D m_originalCenter;
		Vec2D m_center;

		Float m_innerRadius;
		Float m_originalInnerRadius;
		Float m_outerRadius;
		Float m_originalOuterRadius;
};

class ShapeTriangle : public Shape {
	public:
		ShapeTriangle(const Vec2D& point1, const Vec2D& point2, const Vec2D& point3, const Shape::Function& startingShapeFunction, 
					  const Shape::Function& intendedShapeFunction) : Shape(startingShapeFunction, intendedShapeFunction) {

			m_originalPoints[0] = point1;
			m_originalPoints[1] = point2;
			m_originalPoints[2] = point3;
			m_points[0] = point1;
			m_points[1] = point2;
			m_points[2] = point3;
		}

		~ShapeTriangle() {}


		bool isTouching(const Vec2D& particleCenter) const {
			return (isTouchingVertex(particleCenter) || isCenterInside(particleCenter) || intersectsLine(particleCenter));
		}


		void translate(const Vec2D& translation) {
			for (int i = 0; i < 3; i++) {
				m_points[i] = m_points[i] + translation;
			}
		}

		void rotate(const Float& angle) {
			Vec2D center = findTriangleCenter();
			for (int i = 0; i < 3; i++) {
				m_points[i] = VecUtils::rotatePoint(m_points[i], center, angle);
			}
		}

		void scale(const Float& scaleFactor) {
			Vec2D center = findTriangleCenter();
			for (int i = 0; i < 3; i++) {
				Vec2D difference = m_points[i] - center;
				m_points[i] = (difference * scaleFactor) + center;
			}
		}

		void reset() {
			m_points[0] = m_originalPoints[0];
			m_points[1] = m_originalPoints[1];
			m_points[2] = m_originalPoints[2];
			m_shapeFunction = m_originalShapeFunction;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Vec2D point1 = drawOptions.transform(m_points[0]);
			Vec2D point2 = drawOptions.transform(m_points[1]);
			Vec2D point3 = drawOptions.transform(m_points[2]);

			Colour fillColour = getColour(colourPalette);

			Draw::triangle(fillColour, point1, point2, point3);
		}

	private:
		Vec2D m_originalPoints[3];
		Vec2D m_points[3];


		bool isTouchingVertex(const Vec2D& particleCenter) const {
			// Checks whether a vertex is within the moving particle
			for (int i = 0; i < 3; i++) {
				if (VecUtils::distance(particleCenter, m_points[i]) < PARTICLE_RADIUS) {
					return true;
				}
			}

			return false;
		}

		bool isCenterInside(const Vec2D& particleCenter) const {
			// Checks if center is within triangle
			Float totalArea = VecUtils::triangleArea(m_points[0], m_points[1], m_points[2]);
			Float partialAreaSum = 0;

			partialAreaSum += VecUtils::triangleArea(particleCenter, m_points[0], m_points[1]);
			partialAreaSum += VecUtils::triangleArea(particleCenter, m_points[0], m_points[2]);
			partialAreaSum += VecUtils::triangleArea(particleCenter, m_points[1], m_points[2]);

			return (partialAreaSum == totalArea);
		}

		bool intersectsLine(const Vec2D& particleCenter) const {
			Float minDistA = ((2 * VecUtils::triangleArea(particleCenter, m_points[0], m_points[1])) / VecUtils::distance(m_points[0], m_points[1]));
			Float minDistB = ((2 * VecUtils::triangleArea(particleCenter, m_points[0], m_points[2])) / VecUtils::distance(m_points[0], m_points[2]));
			Float minDistC = ((2 * VecUtils::triangleArea(particleCenter, m_points[1], m_points[2])) / VecUtils::distance(m_points[1], m_points[2]));

			Float angle12 = VecUtils::angle3(m_points[0], particleCenter, m_points[1]);
			Float angle13 = VecUtils::angle3(m_points[0], particleCenter, m_points[2]);
			Float angle21 = VecUtils::angle3(m_points[1], particleCenter, m_points[0]);
			Float angle23 = VecUtils::angle3(m_points[1], particleCenter, m_points[2]);
			Float angle31 = VecUtils::angle3(m_points[2], particleCenter, m_points[0]);
			Float angle32 = VecUtils::angle3(m_points[2], particleCenter, m_points[1]);

			if (minDistA < PARTICLE_RADIUS && angle12 < C_PI / 2 && angle21 < C_PI / 2) {
				return true;
			}

			if (minDistB < PARTICLE_RADIUS && angle13 < C_PI / 2 && angle31 < C_PI / 2) {
				return true;
			}

			if (minDistC < PARTICLE_RADIUS && angle23 < C_PI / 2 && angle32 < C_PI / 2) {
				return true;
			}

			return false;
		}


		Vec2D findTriangleCenter() const {
			// solves linear equation system where distance from point 0
			// to center equals that of points 1 and 2 respectively
			Float p1x = m_points[0].x, p1y = m_points[0].y;
			Float p2x = m_points[1].x, p2y = m_points[1].y;
			Float p3x = m_points[2].x, p3y = m_points[2].y;

			// used to avoid division by zero later on
			if (p1y == p2y) {
				std::swap(p1x, p3x);
				std::swap(p1y, p3y);
			}

			Float p1x2 = p1x * p1x, p1y2 = p1y * p1y;
			Float p2x2 = p2x * p2x, p2y2 = p2y * p2y;
			Float p3x2 = p3x * p3x, p3y2 = p3y * p3y;

			Float a = p2x - p1x;
			Float b = p2y - p1y;
			Float c = (Float)0.5 * (p2x2 + p2y2 - p1x2 - p1y2);
			Float d = p3x - p1x;
			Float e = p3y - p1y;
			Float f = (Float)0.5 * (p3x2 + p3y2 - p1x2 - p1y2);

			Float b1 = 1 / b;
			Float denom = d - (e * a * b1);

			Float cx = (f - (e * c * b1)) / denom;
			Float cy = (c - (cx * a)) * b1;

			Vec2D center(cx, cy, Vec2D::CARTESIAN);
			return center;
		}
};

class ShapeRectangle : public Shape {
	public:
		ShapeRectangle(const Vec2D& upperLeftCorner, const Vec2D& size, const Float& angle, const Shape::Function& startingShapeFunction, 
					   const Shape::Function& intendedShapeFunction) : Shape(startingShapeFunction, intendedShapeFunction) {
			m_originalUpperLeftCorner = upperLeftCorner;
			m_upperLeftCorner = upperLeftCorner;
			m_size = size;
			m_originalSize = size;
			m_angle = angle;
			m_originalAngle = m_angle;
		}

		~ShapeRectangle() {}


		bool isTouching(const Vec2D& particleCenter) const {
			Vec2D fakeParticleCenter = getFakeCenter(particleCenter);

			if (fakeParticleCenter.x > m_upperLeftCorner.x                               &&
				fakeParticleCenter.x < m_upperLeftCorner.x + m_size.x                    &&
				fakeParticleCenter.y > m_upperLeftCorner.y            - PARTICLE_RADIUS  &&
				fakeParticleCenter.y < m_upperLeftCorner.y + m_size.y + PARTICLE_RADIUS) {

				return true;
			}

			if (fakeParticleCenter.y > m_upperLeftCorner.y                               &&
				fakeParticleCenter.y < m_upperLeftCorner.y + m_size.y                    &&
				fakeParticleCenter.x > m_upperLeftCorner.x            - PARTICLE_RADIUS  &&
				fakeParticleCenter.x < m_upperLeftCorner.x + m_size.x + PARTICLE_RADIUS) {

				return true;
			}

			Vec2D corners[4];
			corners[0].set(m_upperLeftCorner.x,            m_upperLeftCorner.y           );
			corners[1].set(m_upperLeftCorner.x,            m_upperLeftCorner.y + m_size.y);
			corners[2].set(m_upperLeftCorner.x + m_size.x, m_upperLeftCorner.y           );
			corners[3].set(m_upperLeftCorner.x + m_size.x, m_upperLeftCorner.y + m_size.y);

			for (int i = 0; i < 4; i++) {
				if (VecUtils::distance(fakeParticleCenter, corners[i]) < PARTICLE_RADIUS) {
					return true;
				}
			}

			return false;
		}


		void translate(const Vec2D& translation) {
			m_upperLeftCorner = m_upperLeftCorner + translation;
		}

		void rotate(const Float& angle) {
			m_angle += angle;
		}

		void scale(const Float& scaleFactor) {
			m_size = m_size * scaleFactor;
			m_upperLeftCorner = m_upperLeftCorner + ((m_size * 0.5) * (1 - scaleFactor));
		}

		void reset() {
			m_upperLeftCorner = m_originalUpperLeftCorner;
			m_angle = m_originalAngle;
			m_size = m_originalSize;
			m_shapeFunction = m_originalShapeFunction;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Vec2D realCorners[4];
			Vec2D rectCenter = m_upperLeftCorner + (m_size / (Float)2);

			for (int i = 0; i < 4; i++) {
				Vec2D point(m_upperLeftCorner.x + (i % 2 == 0 ? 0 : m_size.x), m_upperLeftCorner.y + (i % 4 < 2 ? 0 : m_size.y), Vec2D::CARTESIAN);
				realCorners[i] = VecUtils::rotatePoint(point, rectCenter, m_angle);
				realCorners[i] = drawOptions.transform(realCorners[i]);
			}

			Colour fillColour = getColour(colourPalette);

			Draw::triangle(fillColour, realCorners[0], realCorners[2], realCorners[1]);
			Draw::triangle(fillColour, realCorners[3], realCorners[1], realCorners[2]);
		}

	private:
		Vec2D m_originalUpperLeftCorner;
		Vec2D m_upperLeftCorner;

		Float m_originalAngle;
		Float m_angle;

		Vec2D m_size;
		Vec2D m_originalSize;


		Vec2D getFakeCenter(const Vec2D& realCenter) const {
			Vec2D rectCenter = m_upperLeftCorner + (m_size / (Float)2);
			return VecUtils::rotatePoint(realCenter, rectCenter, -m_angle);
		}
			
};

class ShapeRegularPolygon : public Shape {
	public:
		ShapeRegularPolygon(const size_t& nSides, const Vec2D& center, const Float& radius, const Float& angle, 
							const Shape::Function& startingShapeFunction, const Shape::Function& intendedShapeFunction) : 
							Shape(startingShapeFunction, intendedShapeFunction) {

			m_nSides = nSides;
				
			m_originalCenter = center;
			m_center = center;

			m_radius = radius;
			m_originalRadius = radius;

			m_originalAngle = angle;
			m_angle = angle;
		}

		~ShapeRegularPolygon() {}


		bool isTouching(const Vec2D& particleCenter) const {
			Float angleBetweenPoints = 2 * C_PI / m_nSides;

			for (size_t i = 0; i < m_nSides; i++) {
				Float angle1 = (angleBetweenPoints * i) + m_angle;
				Float angle2 = angle1 + angleBetweenPoints;

				Vec2D point1(m_radius, angle1, Vec2D::POLAR);
				Vec2D point2(m_radius, angle2, Vec2D::POLAR);

				point1 = point1 + m_center;
				point2 = point2 + m_center;

				ShapeTriangle subpolygon(m_center, point1, point2, NONE, NONE);
				if (subpolygon.isTouching(particleCenter)) {
					return true;
				}
			}

			return false;
		}


		void translate(const Vec2D& translation) {
			m_center = m_center + translation;
		}

		void rotate(const Float& angle) {
			m_angle += angle;
		}

		void scale(const Float& scaleFactor) {
			m_radius *= scaleFactor;
		}

		void reset() {
			m_center = m_originalCenter;
			m_angle = m_originalAngle;
			m_radius = m_originalRadius;
			m_shapeFunction = m_originalShapeFunction;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Vec2D realCenter = drawOptions.transform(m_center);
				
			Float realRadius = drawOptions.scaleOnly(m_radius);

			Float realAngle = (360 * m_angle) / (2 * C_PI);

			Colour fillColour = getColour(colourPalette);

			Draw::regularPolygon(fillColour, m_nSides, realCenter, realRadius, realAngle);
		}

	private:
		size_t m_nSides;
		
		Vec2D m_originalCenter;
		Vec2D m_center;
			
		Float m_radius;
		Float m_originalRadius;
			
		Float m_originalAngle;
		Float m_angle;
};


class Particle {
	public:
		Particle() {
			m_charge = 1;
		}

		Particle(const Vec2D& position, const int& charge) {
			m_originalPosition = position;
			m_position = position;
			m_charge = charge;
		}

		Particle(const Float& x, const Float y, const int charge) {
			m_position.x = x;
			m_position.y = y;
			m_originalPosition.x = x;
			m_originalPosition.y = y;
			m_charge = charge;
		}

		~Particle() {}


		virtual void reset() = 0;

		void set(const Vec2D& position, int charge) {
			m_position = position;
			m_charge = charge;
		}

		Vec2D getPosition() const {
			return m_position;
		}

		int getCharge() const {
			return m_charge;
		}

		virtual void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const = 0;


	protected:
		Vec2D m_originalPosition;
		Vec2D m_position; // Center of the particle
		Float m_charge;
};

class StaticParticle : public Particle {
	public:
		StaticParticle(const Vec2D& position, const Float& charge) : Particle(position, charge) {}

		StaticParticle(const Float& x, const Float& y, const Float& charge) : Particle(x, y, charge) {}

		~StaticParticle() {}


		void setPosition(const Vec2D& position) {
			m_originalPosition = position;
			m_position = position;
		}

		void invertCharge() {
			m_charge = -m_charge;
		}


		void reset() {
			m_position = m_originalPosition;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Float realParticleRadius = drawOptions.scaleOnly(PARTICLE_RADIUS);
			Float realOutlineThickness = drawOptions.scaleOnly(PARTICLE_LINE_THICKNESS);

			Vec2D realCenterPosition = drawOptions.transform(m_position);

			Colour fillColour;
			switch ((int)m_charge) {
				case -2:
					fillColour = colourPalette.colours.at("staticParticleFillNeg2");
					break;
				case -1:
					fillColour = colourPalette.colours.at("staticParticleFillNeg1");
					break;
				case 0:
					fillColour = colourPalette.colours.at("staticParticleFill0");
					break;
				case 1:
					fillColour = colourPalette.colours.at("staticParticleFillPos1");
					break;
				case 2:
					fillColour = colourPalette.colours.at("staticParticleFillPos2");
					break;
				default:
					if (m_charge > 0) {
						fillColour = colourPalette.colours.at("staticParticleFillOver2");
					}
					else {
						fillColour = colourPalette.colours.at("staticParticleFillUnderNeg2");
					}
					break;
			}

			Float realOutlineInnerRadius = drawOptions.scaleOnly(PARTICLE_RADIUS - PARTICLE_LINE_THICKNESS);
			Float realOutlineOuterRadius = drawOptions.scaleOnly(PARTICLE_RADIUS);

			Draw::circle(fillColour, realCenterPosition, realParticleRadius);
		}
};

class MovingParticle : public Particle {
	public:
		MovingParticle() : Particle() {
			m_mass = 1;
		}

		MovingParticle(const Vec2D& position, const int& charge, const Float& mass = 1) : Particle(position, charge) {
			m_mass = mass;
		}

		~MovingParticle() {}

		void reset() {
			m_velocity.set(0, 0);
			m_position = m_originalPosition;
		}

		void updatePosition(const std::list<StaticParticle>& particleList, const std::vector<Shape*>& shapesVec) {
			Vec2D force;
			Float dt = 1 / (Float)(FPS * GAME_UPDATES_PER_FRAME);

			static const Float c_K = 3000;

			for (auto it = particleList.cbegin(); it != particleList.cend(); it++) {
				Float absForce = c_K * ((m_charge * it->getCharge()) / VecUtils::squaredDistance(m_position, it->getPosition()));
				Float angle = VecUtils::getAngle(m_position, it->getPosition());
				force.x += absForce * COS(angle);
				force.y += absForce * SIN(angle);
			}

			for (auto it = shapesVec.cbegin(); it != shapesVec.cend(); it++) {
				if ((*it)->isTouching(m_position)) {
					static const Float c_B = 1.5;

					Float magneticFieldDirection = 0;
					switch ((*it)->getShapeFunction()) {
						case Shape::MAGNETIC_FIELD_OUT:
							magneticFieldDirection = c_B;
							break;

						case Shape::MAGNETIC_FIELD_IN:
							magneticFieldDirection = -c_B;
							break;

						default:
							break;
					}

					if (magneticFieldDirection != 0) {
						Vec2D localForce(m_velocity.y, -m_velocity.x);
						localForce = localForce * m_charge * magneticFieldDirection;

						force = force + localForce;
					}
				}
			}

			m_acceleration = force / m_mass;
			m_velocity = m_velocity + (m_acceleration * dt);
			m_position = m_position + m_velocity;
		}


		MovingParticle& operator=(const MovingParticle& p) {
			m_position = p.m_position;
			m_charge = p.m_charge;
			m_mass = p.m_mass;
			m_originalPosition = p.m_originalPosition;
			m_velocity = p.m_velocity;
			m_acceleration = p.m_acceleration;

			return *this;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Float realParticleRadius = drawOptions.scaleOnly(PARTICLE_RADIUS);
			Float realOutlineThickness = drawOptions.scaleOnly(PARTICLE_LINE_THICKNESS);

			Vec2D realCenterPosition = drawOptions.transform(m_position);

			Colour fillColour = colourPalette.colours.at("movingParticleFill");
			Colour symbolColour = colourPalette.colours.at("movingParticleSymbol");

			Draw::circle(fillColour, realCenterPosition, realParticleRadius);

			Vec2D symbolPositionL(realCenterPosition.x - (realParticleRadius / 2), realCenterPosition.y);
			Vec2D symbolPositionR(realCenterPosition.x + (realParticleRadius / 2), realCenterPosition.y);
			Draw::line(symbolColour, symbolPositionL, symbolPositionR);

			if (m_charge > 0) {
				Vec2D symbolPositionU(realCenterPosition.x, realCenterPosition.y - (realParticleRadius / 2));
				Vec2D symbolPositionD(realCenterPosition.x, realCenterPosition.y + (realParticleRadius / 2));
				Draw::line(symbolColour, symbolPositionU, symbolPositionD);
			}
		}

	private:
		Float m_mass;
		Vec2D m_velocity;
		Vec2D m_acceleration;
};


class Goal {
	public:
		Goal(const Vec2D& position, const int& radius, const bool& isEnabled) {
			m_position = position;
			m_originalPosition = position;

			m_radius = radius;

			m_isEnabled = isEnabled;
			m_initiallyEnabled = isEnabled;
		}

		~Goal() {}


		void set(const Vec2D& position, const int& radius) {
			m_position = position;
			m_radius = radius;
		}

		Vec2D getPosition() const {
			return m_position;
		}

		int getRadius() const {
			return m_radius;
		}


		bool isTouching(const MovingParticle& p) const {
			return ((VecUtils::distance(p.getPosition(), m_position) <= m_radius) && m_isEnabled);
		}


		Goal& operator=(const Goal& goal) {
			m_position = goal.m_position;
			m_radius = goal.m_radius;
			return *this;
		}


		void rotate(const Float& angle) {
			// hello there >:)
		}

		void translate(const Vec2D& translation) {
			m_position = m_position + translation;
		}

		void toggle(const bool& shouldToggle) {
			m_isEnabled = m_isEnabled ^ shouldToggle;
		}

		void applyAnim(const Animation::Movement& movement) {
			translate(movement.move.translation);
			toggle(movement.move.toggle);
		}


		void reset() {
			m_position = m_originalPosition;
			m_isEnabled = m_initiallyEnabled;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			if (m_isEnabled) {
				Float realGoalRadius = drawOptions.scaleOnly(m_radius);

				Vec2D realGoalPosition = drawOptions.transform(m_position);

				Colour fillColour = colourPalette.colours.at("goalFill");

				Draw::circle(fillColour, realGoalPosition, realGoalRadius);
			}
		}

	private:
		Vec2D m_position, m_originalPosition;
		int m_radius;

		bool m_isEnabled, m_initiallyEnabled;
};


class Button {
	public:
		Button() {}

		Button(const Vec2D& upperLeftCorner, const Vec2D& size, const std::string& text, const int& textSize, const bool& isDisabled) {
			m_upperLeftCorner = upperLeftCorner;
			m_size = size;
			m_text = text;
			m_textSize = textSize;
			m_isDisabled = isDisabled;
		}

		~Button() {}


		void set(const Vec2D& upperLeftCorner, const Vec2D& size, const std::string& text, const int& textSize, const bool& isDisabled) {
			m_upperLeftCorner = upperLeftCorner;
			m_size = size;
			m_text = text;
			m_textSize = textSize;
			m_isDisabled = isDisabled;
		}

		void setDisabled(const bool& disabled) {
			m_isDisabled = disabled;
		}

		void setText(const std::string& text) {
			m_text = text;
		}


		bool isMouseOver(const DrawOptions& drawOptions) const {
			Vec2D mousePos = Input::getMousePosition(drawOptions);

			return ((mousePos.x >= m_upperLeftCorner.x               ) && 
					(mousePos.y >= m_upperLeftCorner.y               ) && 
					(mousePos.x <= m_upperLeftCorner.x + m_size.x    ) && 
					(mousePos.y <= m_upperLeftCorner.y + m_size.y    ));
		}

		std::string isReleased(const char& mouseInput, const DrawOptions& drawOptions) const {
			return ((isMouseOver(drawOptions) && Input::isMouseReleased(mouseInput) && !m_isDisabled) ? m_text : "\n");
		}

		bool isDisabled() const {
			return m_isDisabled;
		}

		std::string getText() const {
			return m_text;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			Vec2D realUpperLeftCorner = drawOptions.transform(m_upperLeftCorner);
			Vec2D realSize = drawOptions.scaleOnly(m_size);

			Float virtualTextOffset = (m_size.y - m_textSize) / 2;
			Vec2D virtualTextOffsetVec(virtualTextOffset, virtualTextOffset, Vec2D::CARTESIAN);
			Vec2D virtualTextPos = m_upperLeftCorner + virtualTextOffsetVec;
			Vec2D realTextPos = drawOptions.transform(virtualTextPos);

			int realTextSize = drawOptions.scaleOnly(m_textSize);

			Colour buttonColour = (isMouseOver(drawOptions) || m_isDisabled) ? colourPalette.colours.at("buttonHover") : colourPalette.colours.at("buttonRegular");
			Colour textColour = !m_isDisabled ? colourPalette.colours.at("buttonText") : colourPalette.colours.at("buttonTextDisabled");

			Draw::rectangle(buttonColour, realUpperLeftCorner, realSize);
			Draw::text(textColour, realTextPos, realTextSize, m_text);
		}

	private:
		Vec2D m_upperLeftCorner;
		Vec2D m_size;
		std::string m_text;
		int m_textSize;
		bool m_isDisabled;
};


class NoticeText {
	public:
		typedef enum Type {
			NORMAL = 0,
			CONFIRMATION,
			WARNING,
			ERROR
		};

		NoticeText(const NoticeText::Type& noticeType, const std::string& text, const Vec2D& position, 
				   const int& fontSize, const Float& timeAppearing, const Float& timeVanishing = 0) {

			m_noticeType = noticeType;
			m_text = text;
			m_fontSize = fontSize;
			m_position = position;
			m_timeAppearing = timeAppearing;
			m_timeVanishing = timeVanishing;
			m_timeCounter = 0;
		}


		void update() {
			m_timeCounter += (Float)1 / FPS;
		}


		bool isShown() const {
			return (m_timeCounter <= m_timeAppearing + m_timeVanishing);
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) {
			if (m_timeCounter <= m_timeAppearing + m_timeVanishing) {
				Vec2D realPosition = drawOptions.transform(m_position);
				int realFontSize = drawOptions.scaleOnly(m_fontSize);

				Colour textColour;
				switch (m_noticeType) {
					case NORMAL:
						textColour = colourPalette.colours.at("normalText");
						break;

					case CONFIRMATION:
						textColour = colourPalette.colours.at("successText");
						break;

					case WARNING:
						textColour = colourPalette.colours.at("warningText");
						break;

					case ERROR:
						textColour = colourPalette.colours.at("failureText");
						break;
				}

				if (m_timeCounter > m_timeAppearing) {
					unsigned int subtractVal = ROUND(((m_timeCounter - m_timeAppearing) / m_timeVanishing) * 255);
					textColour.a -= subtractVal;
				}

				Draw::text(textColour, realPosition, realFontSize, m_text);

				update();
			}
		}

	private:
		std::string m_text;
		int m_fontSize;
		Vec2D m_position;

		Float m_timeAppearing;
		Float m_timeVanishing;
		Float m_timeCounter;

		NoticeText::Type m_noticeType;
};


class SavesManager {
	public:
		SavesManager() = delete;

		static void store(const std::list<StaticParticle>& staticParticlesList, const std::string& fileName) {
			pugi::xml_document outputFile;
			outputFile.append_child("save");
			pugi::xml_node saveNode = outputFile.child("save");

			for (auto it = staticParticlesList.cbegin(); it != staticParticlesList.cend(); it++) {
				saveNode.append_child("particle");
				pugi::xml_node_iterator particleNode = saveNode.end();
				particleNode--;

				Vec2D position = it->getPosition();
				std::string positionStr = std::to_string(position.x) + "," + std::to_string(position.y);
				std::string chargeStr = std::to_string(it->getCharge());

				particleNode->append_attribute("pos") = positionStr.c_str();
				particleNode->append_attribute("charge") = chargeStr.c_str();
			}

			outputFile.save_file(fileName.c_str());
		}

		static void load(const std::string& savePath, std::list<StaticParticle>& staticParticleList) {
			staticParticleList.clear();

			pugi::xml_document file;
			pugi::xml_parse_result stageXML = file.load_file(savePath.c_str());

			pugi::xml_node rootNode = file.child("save");

			for (auto it = rootNode.begin(); it != rootNode.end(); it++) {
				Vec2D position = VecUtils::stringToVec(it->attribute("pos").as_string());
				Float charge = AS_FLOAT(it->attribute("charge"));

				StaticParticle p(position, charge);
				staticParticleList.push_back(p);
			}
		}
};

class Stage {
	friend class LoadStageXML;

	public:
		Stage(const std::string& stageName = "") {
			m_stageName = stageName.substr(stageName.find_last_of("/") + 1);
			m_stageName = m_stageName.substr(0, m_stageName.length() - XML_FILE_EXTENSION.length());
			m_nonLoopingTogglesIndex = 0;

			m_loadStoreNotice = nullptr;
		}

		Stage(const std::string& stageName, const std::list<StaticParticle>& staticParticles,
			  const std::vector<MovingParticle> movingParticles, std::vector<Goal> goals, std::vector<Shape*> shapes,
			  std::vector<Shape*> magneticFields, std::vector<Animation*> animations, std::vector<Toggle> nonLoopingToggles) :
			m_stageName(stageName), m_staticParticles(staticParticles), m_movingParticles(movingParticles), m_goals(goals),
			m_shapes(shapes), m_magneticFields(magneticFields), m_animations(animations), m_nonLoopingToggles(nonLoopingToggles) {

			m_nonLoopingTogglesIndex = 0;
			m_loadStoreNotice = nullptr;
		}

		~Stage() {
			for (auto it = m_shapes.begin(); it != m_shapes.end(); it++) {
				delete (*it);
			}

			for (auto it = m_animations.begin(); it != m_animations.end(); it++) {
				delete (*it);
			}

			if (m_loadStoreNotice != nullptr) {
				delete m_loadStoreNotice;
			}
		}

			
		void update(unsigned long& updateCounter) {
			for (int i = 0; i < GAME_UPDATES_PER_FRAME; i++) {
				for (auto it = m_movingParticles.begin(); it != m_movingParticles.end(); it++) {
					if (!isMovingParticleTouchingGoal(it)) {
						it->updatePosition(m_staticParticles, m_magneticFields);
					}
				}

				for (auto it = m_animations.begin(); it != m_animations.end(); it++) {
					Animation::Movement movement = (*it)->calculateAnim(updateCounter);

					if (movement.animationApplied) {
						switch (movement.target.tType) {
							case Animation::Target::GOAL:
								m_goals[movement.target.index].applyAnim(movement);
								break;

							case Animation::Target::OBSTACLE:
								m_shapes[movement.target.index]->applyAnim(movement);
								break;

							case Animation::Target::MAGNETIC_FIELD:
								m_magneticFields[movement.target.index]->applyAnim(movement);

							default:
								break;
						}
					}
				}

				if (!m_nonLoopingToggles.empty() && m_nonLoopingTogglesIndex < m_nonLoopingToggles.size()) {
					while (m_nonLoopingToggles[m_nonLoopingTogglesIndex].getTime() == updateCounter) {
						Animation::Movement movement = m_nonLoopingToggles[m_nonLoopingTogglesIndex].calculateAnim(updateCounter);

						if (movement.animationApplied) {
							switch (movement.target.tType) {
								case Animation::Target::GOAL:
									m_goals[movement.target.index].applyAnim(movement);
									break;

								case Animation::Target::OBSTACLE:
									m_shapes[movement.target.index]->applyAnim(movement);
									break;

								case Animation::Target::MAGNETIC_FIELD:
									m_magneticFields[movement.target.index]->applyAnim(movement);

								default:
									break;
							}
						}

						m_nonLoopingTogglesIndex++;
						if (m_nonLoopingTogglesIndex >= m_nonLoopingToggles.size()) {
							break;
						}
					}
				}

				if (isMovingParticleColliding()) {
					break;
				}

				updateCounter++;
			}
		}

		bool isMovingParticleColliding() const {
			for (auto itParticle = m_movingParticles.cbegin(); itParticle != m_movingParticles.cend(); itParticle++) {
				bool flag = false;
				for (auto itShape = m_shapes.cbegin(); itShape != m_shapes.cend(); itShape++) {
					if (((*itShape)->getShapeFunction() == Shape::OBSTACLE) && (*itShape)->isTouching(itParticle->getPosition())) {
						flag = true;
					}
				}

				if (flag) {
					return true;
				}
			}

			return false;
		}

		bool isMovingParticleTouchingGoal(const std::vector<MovingParticle>::const_iterator& itParticle) const {
			for (auto itGoal = m_goals.cbegin(); itGoal != m_goals.cend(); itGoal++) {
				if (itGoal->isTouching(*itParticle)) {
					return true;
				}
			}

			return false;
		}

		bool allMovingParticlesTouchingGoals() const {
			for (auto it = m_movingParticles.cbegin(); it != m_movingParticles.cend(); it++) {
				if (!isMovingParticleTouchingGoal(it)) {
					return false;
				}
			}

			return true;
		}

		bool anyMovingParticles() const {
			return (m_movingParticles.size() > 0);
		}
			
			
		void reset() {
			for (auto it = m_movingParticles.begin(); it != m_movingParticles.end(); it++) {
				it->reset();
			}

			for (auto it = m_goals.begin(); it != m_goals.end(); it++) {
				it->reset();
			}

			for (auto it = m_shapes.begin(); it != m_shapes.end(); it++) {
				(*it)->reset();
			}

			for (auto it = m_magneticFields.begin(); it != m_magneticFields.end(); it++) {
				(*it)->reset();
			}

			m_nonLoopingTogglesIndex = 0;
		}

		void addStaticParticle(const StaticParticle& p) {
			m_staticParticles.push_back(p);
		}

		void removeStaticParticles(const Vec2D& mousePos) {
			std::list<StaticParticle>::iterator it = m_staticParticles.begin();

			while (it != m_staticParticles.end()) {
				if (VecUtils::distance(mousePos, it->getPosition()) < PARTICLE_RADIUS) {
					it = m_staticParticles.erase(it);
				}

				else {
					it++;
				}
			}
		}


		void storeReplay() {
			std::time_t currentTime;
			std::time(&currentTime);

			std::string fileName = SAVES_DIR + "/" + std::to_string(currentTime) + "_" + m_stageName + ".xml";
				
			SavesManager::store(m_staticParticles, fileName);

			if (m_loadStoreNotice != nullptr) {
				delete m_loadStoreNotice;
			}
			static const Vec2D d_noticePosition(50, 550);
			m_loadStoreNotice = new NoticeText(NoticeText::NORMAL, "Stored at " + fileName, d_noticePosition, TEXT_FONT_SIZE_NORMAL, 1, 0.5);
		}

		void loadReplay() {
			std::string inputFileLocation = Input::getDroppedFileLocation();
			SavesManager::load(inputFileLocation, m_staticParticles);
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			for (auto it = m_goals.cbegin(); it != m_goals.cend(); it++) {
				it->draw(drawOptions, colourPalette);
			}

			for (auto it = m_shapes.cbegin(); it != m_shapes.cend(); it++) {
				if ((*it)->getShapeFunction() != Shape::NONE) {
					(*it)->draw(drawOptions, colourPalette);
				}
			}

			for (auto it = m_magneticFields.cbegin(); it != m_magneticFields.cend(); it++) {
				if ((*it)->getShapeFunction() != Shape::NONE) {
					(*it)->draw(drawOptions, colourPalette);
				}
			}

			for (auto it = m_staticParticles.cbegin(); it != m_staticParticles.cend(); it++) {
				it->draw(drawOptions, colourPalette);
			}

			for (auto it = m_movingParticles.cbegin(); it != m_movingParticles.cend(); it++) {
				it->draw(drawOptions, colourPalette);
			}

			if (m_loadStoreNotice != nullptr) {
				m_loadStoreNotice->draw(drawOptions, colourPalette);
			}
		}

	private:
		std::string m_stageName;

		std::list<StaticParticle> m_staticParticles;
		std::vector<MovingParticle> m_movingParticles;
		std::vector<Goal> m_goals;
		std::vector<Shape*> m_shapes;
		std::vector<Shape*> m_magneticFields;

		std::vector<Animation*> m_animations;
		std::vector<Toggle> m_nonLoopingToggles;  // Stored separately for optimisation
		size_t m_nonLoopingTogglesIndex;

		NoticeText* m_loadStoreNotice;
};

class LoadStageXML {
	public:
		typedef struct TutorialReturn {
			std::string title;
			std::vector<std::string> text;
			std::list<StaticParticle> staticParticles;
			unsigned long loopTime;
		};

		LoadStageXML() = delete;

		static Stage* loadStage(const std::string& stageName, const bool& mirrorModeOn, const bool& invertMovingParticleCharges) {
			Stage* stage = new Stage(stageName);

			if (stageName != "") {
				pugi::xml_document file;
				pugi::xml_parse_result stageXML = file.load_file(stageName.c_str());

				pugi::xml_node rootNode = file.child("stage");

				std::unordered_map<unsigned int, Vec2D> points = getPoints(rootNode.child("points"));

				addMovingParticles(rootNode.child("movingParticles"), points, stage, mirrorModeOn, invertMovingParticleCharges);

				std::unordered_map<id_val, Animation::Target> indexes;

				addGoals(rootNode.child("goals"), points, indexes, stage, mirrorModeOn);
				addShapes("obstacles", rootNode.child("obstacles"), points, indexes, stage, mirrorModeOn);
				addShapes("magneticFields", rootNode.child("magneticFields"), points, indexes, stage, mirrorModeOn);

				addAnimations(rootNode.child("animations"), points, indexes, stage, mirrorModeOn);
			}

			return stage;
		}

		static LoadStageXML::TutorialReturn loadTutorialSpecificData(const std::string& stagePath) {
			pugi::xml_document file;
			pugi::xml_parse_result stageXML = file.load_file(stagePath.c_str());
			pugi::xml_node rootNode = file.child("stage");

			LoadStageXML::TutorialReturn tutorialReturn;

			tutorialReturn.title = rootNode.child("title").attribute("val").as_string();

			pugi::xml_node textNode = rootNode.child("text");
			for (auto it = textNode.begin(); it != textNode.end(); it++) {
				tutorialReturn.text.push_back(it->attribute("val").as_string());
			}

			tutorialReturn.staticParticles = addStaticParticles(rootNode.child("staticParticles"));

			tutorialReturn.loopTime = rootNode.child("loopTime").attribute("val").as_ullong();

			return tutorialReturn;
		}

	private:
		static std::unordered_map<unsigned int, Vec2D> getPoints(const pugi::xml_node& pointsNode) {
			std::unordered_map<unsigned int, Vec2D> points;

			for (auto it = pointsNode.begin(); it != pointsNode.end(); it++) {
				id_val pointID = it->attribute("id").as_uint();

				Vec2D point = VecUtils::stringToVec(it->attribute("pos").as_string());

				points.insert({ pointID, point });
			}

			return points;
		}

		static void addMovingParticles(const pugi::xml_node& movingParticlesNode, const std::unordered_map<unsigned int, Vec2D>& points, 
									   Stage* stage, const bool& mirrorModeOn, const bool& invertMovingParticleCharges) {

			for (auto it = movingParticlesNode.begin(); it != movingParticlesNode.end(); it++) {

				Vec2D position = VecUtils::decipherStringToVec(it->attribute("pos").as_string(), points);
				position.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - position.x) : position.x);
					
				Float charge = AS_FLOAT(it->attribute("charge"));
				charge *= (invertMovingParticleCharges ? -1 : 1);

				MovingParticle p(position, charge);
				stage->m_movingParticles.push_back(p);
			}
		}

		static std::list<StaticParticle> addStaticParticles(const pugi::xml_node& staticParticlesNode) {
			std::list<StaticParticle> staticParticles;
			std::unordered_map<unsigned int, Vec2D> points;

			for (auto it = staticParticlesNode.begin(); it != staticParticlesNode.end(); it++) {
				Vec2D position = VecUtils::decipherStringToVec(it->attribute("pos").as_string(), points);
				Float charge = AS_FLOAT(it->attribute("charge"));

				StaticParticle p(position, charge);
				staticParticles.push_back(p);
			}

			return staticParticles;
		}

		static void addGoals(const pugi::xml_node& goalsNode, const std::unordered_map<unsigned int, Vec2D>& points,
							 std::unordered_map<id_val, Animation::Target>& indexes, Stage* stage, const bool& mirrorModeOn) {

			for (auto it = goalsNode.begin(); it != goalsNode.end(); it++) {
				id_val id = it->attribute("id").as_uint();
				Vec2D position = VecUtils::decipherStringToVec(it->attribute("pos").as_string(), points);
				Float radius = AS_FLOAT(it->attribute("radius"));
				position.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - position.x) : position.x);

				Animation::Target target = { Animation::Target::GOAL, stage->m_goals.size() };
				indexes.insert({ id, target });

				std::string enabled = it->attribute("enabled").as_string();
				bool function = !(enabled == "no");

				Goal g(position, radius, function);
				stage->m_goals.push_back(g);
			}
		}


		static void addCircle(const pugi::xml_node& circleNode, 
							  const std::unordered_map<unsigned int, Vec2D>& points,
							  std::vector<Shape*>& shapesVec, const Shape::Function& startingShapeFunction,
							  const Shape::Function& intendedShapeFunction, const bool& mirrorModeOn) {

			Vec2D center = VecUtils::decipherStringToVec(circleNode.attribute("center").as_string(), points);
			Float radius = AS_FLOAT(circleNode.attribute("radius"));
			center.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - center.x) : center.x);

			shapesVec.push_back(new ShapeCircle(center, radius, startingShapeFunction, intendedShapeFunction));
		}

		static void addBagel(const pugi::xml_node& bagelNode, 
							 const std::unordered_map<unsigned int, Vec2D>& points,
							 std::vector<Shape*>& shapesVec, const Shape::Function& startingShapeFunction,
							 const Shape::Function& intendedShapeFunction, const bool& mirrorModeOn) {

			Vec2D center = VecUtils::decipherStringToVec(bagelNode.attribute("center").as_string(), points);
			Float innerRadius = AS_FLOAT(bagelNode.attribute("innerRadius"));
			Float outerRadius = AS_FLOAT(bagelNode.attribute("outerRadius"));
			center.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - center.x) : center.x);

			shapesVec.push_back(new ShapeBagel(center, innerRadius, outerRadius, startingShapeFunction, intendedShapeFunction));
		}

		static void addRectangle(const pugi::xml_node& rectNode, 
								 const std::unordered_map<unsigned int, Vec2D>& points,
								 std::vector<Shape*>& shapesVec, const Shape::Function& startingShapeFunction,
								 const Shape::Function& intendedShapeFunction, const bool mirrorModeOn) {

			Vec2D upperLeftCorner = VecUtils::decipherStringToVec(rectNode.attribute("upperLeftCorner").as_string(), points);
			Vec2D size = VecUtils::decipherStringToVec(rectNode.attribute("size").as_string(), points);
			Float angle = -AS_FLOAT(rectNode.attribute("angle")) / 360 * 2 * C_PI;
			upperLeftCorner.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - upperLeftCorner.x - size.x) : upperLeftCorner.x);
			angle *= (mirrorModeOn ? -1 : 1);

			shapesVec.push_back(new ShapeRectangle(upperLeftCorner, size, angle, startingShapeFunction, intendedShapeFunction));
		}

		static void addTriangle(const pugi::xml_node& triangleNode, 
								const std::unordered_map<unsigned int, Vec2D>& points,
								std::vector<Shape*>& shapesVec, const Shape::Function& startingShapeFunction,
								const Shape::Function& intendedShapeFunction, const bool& mirrorModeOn) {

			Vec2D point1 = VecUtils::decipherStringToVec(triangleNode.attribute("point1").as_string(), points);
			Vec2D point2 = VecUtils::decipherStringToVec(triangleNode.attribute("point2").as_string(), points);
			Vec2D point3 = VecUtils::decipherStringToVec(triangleNode.attribute("point3").as_string(), points);
			point1.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - point1.x) : point1.x);
			point2.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - point2.x) : point2.x);
			point3.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - point3.x) : point3.x);

			shapesVec.push_back(new ShapeTriangle(point1, point2, point3, startingShapeFunction, intendedShapeFunction));
		}

		static void addRegularPolygon(const pugi::xml_node& regularPolygonNode,
									  const std::unordered_map<unsigned int, Vec2D>& points,
									  std::vector<Shape*>& shapesVec, const Shape::Function& startingShapeFunction,
									  const Shape::Function& intendedShapeFunction, const bool& mirrorModeOn) {

			size_t nSides = regularPolygonNode.attribute("nSides").as_uint();

			Vec2D center = VecUtils::decipherStringToVec(regularPolygonNode.attribute("center").as_string(), points);
			center.x = (mirrorModeOn ? (VIRTUAL_SCREEN_WIDTH - center.x) : center.x);

			Float radius = AS_FLOAT(regularPolygonNode.attribute("radius"));

			Float angle = AS_FLOAT(regularPolygonNode.attribute("angle"));
			angle *= (mirrorModeOn ? -1 : 1);

			shapesVec.push_back(new ShapeRegularPolygon(nSides, center, radius, angle, startingShapeFunction, intendedShapeFunction));
		}

		static void addShapes(const std::string& shapeListType, const pugi::xml_node& shapesNode,
							  const std::unordered_map<unsigned int, Vec2D>& points,
							  std::unordered_map<id_val, Animation::Target>& indexes, Stage* stage, const bool& mirrorModeOn) {

			typedef enum ShapeType {
				CIRCLE = 0,
				BAGEL,
				TRIANGLE,
				RECTANGLE,
				REGULAR_POLYGON
			};

			static const std::unordered_map<std::string, ShapeType> c_stringToShapeType = {
				{"circle"        , CIRCLE},
				{"bagel"         , BAGEL},
				{"triangle"      , TRIANGLE},
				{"rectangle"     , RECTANGLE},
				{"regularPolygon", REGULAR_POLYGON}
			};

			std::vector<Shape*>* listPtr = nullptr;
			if (shapeListType == "obstacles") {
				listPtr = &stage->m_shapes;
			}
			else if (shapeListType == "magneticFields") {
				listPtr = &stage->m_magneticFields;
			}

			for (auto it = shapesNode.begin(); it != shapesNode.end(); it++) {
				Animation::Target::Type targetType;
				if (shapeListType == "obstacles") {
					targetType = Animation::Target::OBSTACLE;
				}
				else if (shapeListType == "magneticFields") {
					targetType = Animation::Target::MAGNETIC_FIELD;
				}

				id_val id = it->attribute("id").as_uint();
				Animation::Target target = { targetType, stage->m_shapes.size() };
				indexes.insert({ id, target });

				std::string enabled = it->attribute("enabled").as_string();

				Shape::Function notDisabledFunction = Shape::NONE;
				if (shapeListType == "obstacles") {
					notDisabledFunction = Shape::OBSTACLE;
				}
				else if (shapeListType == "magneticFields") {
					std::string direction = it->attribute("direction").as_string();
					if (direction == "in") {
						notDisabledFunction = Shape::MAGNETIC_FIELD_IN;
					}
					else if (direction == "out") {
						notDisabledFunction = Shape::MAGNETIC_FIELD_OUT;
					}
				}

				Shape::Function function = (enabled == "no") ? Shape::NONE : notDisabledFunction;

				switch (c_stringToShapeType.at(it->name())) {
					case CIRCLE:
						addCircle(*it, points, *listPtr, function, notDisabledFunction, mirrorModeOn);
						break;

					case BAGEL:
						addBagel(*it, points, *listPtr, function, notDisabledFunction, mirrorModeOn);
						break;

					case RECTANGLE:
						addRectangle(*it, points, *listPtr, function, notDisabledFunction, mirrorModeOn);
						break;

					case TRIANGLE:
						addTriangle(*it, points, *listPtr, function, notDisabledFunction, mirrorModeOn);
						break;
						
					case REGULAR_POLYGON:
						addRegularPolygon(*it, points, *listPtr, function, notDisabledFunction, mirrorModeOn);
						break;

					default:
						break;
				}
			}
		}


		static std::vector<Float> stringToFloatVector(const std::string& str) {
			std::vector<Float> vals;

			size_t previousCommaPos = 0;
			while (str.find(',', previousCommaPos) != std::string::npos) {
				size_t currentCommaPos = str.find(',', previousCommaPos);
				vals.push_back(std::stof(str.substr(previousCommaPos, currentCommaPos)));

				previousCommaPos = currentCommaPos + 1;
			}

			vals.push_back(std::stof(str.substr(previousCommaPos, str.length())));

			return vals;
		}

		static std::vector<Float> stringToSpeedVals(const std::string& speedValsStr) {
			std::vector<Float> speedVals = stringToFloatVector(speedValsStr);
				
			Float valsSum = 0;

			for (size_t i = 0; i < speedVals.size(); i++) {
				valsSum += speedVals[i];
			}

			for (size_t i = 0; i < speedVals.size(); i++) {
				speedVals[i] /= valsSum;;
			}

			return speedVals;
		}

		static void addLinealTranslation(const pugi::xml_node& translationNode, const std::unordered_map<unsigned int, Vec2D>& points,
										 const std::unordered_map<id_val, Animation::Target>& indexes, 
										 Stage* stage, const bool& mirrorModeOn) {

			id_val targetID = translationNode.attribute("id").as_uint();
			Animation::Target target = indexes.at(targetID);

			unsigned long start = translationNode.attribute("startTime").as_ullong();
			unsigned long end = translationNode.attribute("endTime").as_ullong();
			unsigned long loop = translationNode.attribute("loopEvery").as_ullong();

			Vec2D endPoint = VecUtils::decipherStringToVec(translationNode.attribute("endPoint").as_string(), points);
			endPoint.x *= (mirrorModeOn ? -1 : 1);

			std::vector<Float> speedVals = stringToSpeedVals(translationNode.attribute("speedParams").as_string());

			stage->m_animations.push_back(new LinealTranslate(endPoint, speedVals, start, end, loop, target));
		}

		static void addBezierTranslation(const pugi::xml_node& translationNode, const std::unordered_map<unsigned int, Vec2D>& points,
										 const std::unordered_map<id_val, Animation::Target>& indexes, 
										 Stage* stage, const bool& mirrorModeOn) {

			id_val targetID = translationNode.attribute("id").as_uint();
			Animation::Target target = indexes.at(targetID);
			static const Vec2D c_zeroVec(0, 0, Vec2D::CARTESIAN);

			unsigned long start = translationNode.attribute("startTime").as_ullong();
			unsigned long end = translationNode.attribute("endTime").as_ullong();
			unsigned long loop = translationNode.attribute("loopEvery").as_ullong();

			std::vector<Vec2D> controlPoints = VecUtils::decipherStringToVecArray(translationNode.attribute("bezierParams").as_string(), points);
			controlPoints.insert(controlPoints.begin(), c_zeroVec);

			if (mirrorModeOn) {
				for (size_t i = 0; i < controlPoints.size(); i++) {
					controlPoints[i].x *= -1;
				}
			}

			std::vector<Float> speedVals = stringToSpeedVals(translationNode.attribute("speedParams").as_string());

			stage->m_animations.push_back(new BezierTranslate(controlPoints, speedVals, start, end, loop, target));
		}

		static void addCircleTranslation(const pugi::xml_node& translationNode, const std::unordered_map<unsigned int, Vec2D>& points,
										 const std::unordered_map<id_val, Animation::Target>& indexes, 
										 Stage* stage, const bool& mirrorModeOn) {

			id_val targetID = translationNode.attribute("id").as_uint();
			Animation::Target target = indexes.at(targetID);

			unsigned long start = translationNode.attribute("startTime").as_ullong();
			unsigned long end = translationNode.attribute("endTime").as_ullong();
			unsigned long loop = translationNode.attribute("loopEvery").as_ullong();

			Float radius = AS_FLOAT(translationNode.attribute("radius"));
			Float startAngle = AS_FLOAT(translationNode.attribute("startAngle")) * (-2 * C_PI / 360);
			Float endAngle = AS_FLOAT(translationNode.attribute("endAngle")) * (-2 * C_PI / 360);
			startAngle = (mirrorModeOn ? (C_PI - startAngle) : startAngle);
			endAngle = (mirrorModeOn ? (C_PI - endAngle) : endAngle);

			std::vector<Float> speedVals = stringToSpeedVals(translationNode.attribute("speedParams").as_string());

			stage->m_animations.push_back(new CircleTranslate(startAngle, endAngle, radius, speedVals, start, end, loop, target));
		}

		static void addTranslation(const pugi::xml_node& translationNode, const std::unordered_map<unsigned int, Vec2D>& points,
								   const std::unordered_map<id_val, Animation::Target>& indexes, 
								   Stage* stage, const bool mirrorModeOn) {

			static const std::string c_lineal = "lineal", c_bezier = "bezier", c_circle = "circle";

			if (translationNode.attribute("type").as_string() == c_lineal) {
				addLinealTranslation(translationNode, points, indexes, stage, mirrorModeOn);
			}

			else if (translationNode.attribute("type").as_string() == c_bezier) {
				addBezierTranslation(translationNode, points, indexes, stage, mirrorModeOn);
			}

			else if (translationNode.attribute("type").as_string() == c_circle) {
				addCircleTranslation(translationNode, points, indexes, stage, mirrorModeOn);
			}
		}

		static void addRotation(const pugi::xml_node& rotationNode, const std::unordered_map<id_val, Animation::Target>& indexes, 
								Stage* stage, const bool& mirrorModeOn) {

			id_val targetID = rotationNode.attribute("id").as_uint();
			Animation::Target target = indexes.at(targetID);

			unsigned long start = rotationNode.attribute("startTime").as_ullong();
			unsigned long end = rotationNode.attribute("endTime").as_ullong();
			unsigned long loop = rotationNode.attribute("loopEvery").as_ullong();

			Float angle = -AS_FLOAT(rotationNode.attribute("angle")) * 2 * C_PI / 360;
			angle *= (mirrorModeOn ? -1 : 1);

			std::vector<Float> speedVals = stringToSpeedVals(rotationNode.attribute("speedParams").as_string());

			stage->m_animations.push_back(new Rotate(angle, speedVals, start, end, loop, target));
		}

		static void addToggle(const pugi::xml_node& toggleNode,
							  const std::unordered_map<id_val, Animation::Target>& indexes, Stage* stage) {

			id_val targetID = toggleNode.attribute("id").as_uint();
			Animation::Target target = indexes.at(targetID);

			unsigned long time = toggleNode.attribute("time").as_ullong();
			unsigned long loop = toggleNode.attribute("loopEvery").as_ullong();

			if (loop == 0) {
				Toggle toggle(time, loop, target);
				stage->m_nonLoopingToggles.push_back(toggle);
			}
			else {
				stage->m_animations.push_back(new Toggle(time, loop, target));
			}
		}

		static void addScale(const pugi::xml_node& rotationNode,
							 const std::unordered_map<id_val, Animation::Target>& indexes, Stage* stage) {

			id_val targetID = rotationNode.attribute("id").as_uint();
			Animation::Target target = indexes.at(targetID);

			unsigned long start = rotationNode.attribute("startTime").as_ullong();
			unsigned long end = rotationNode.attribute("endTime").as_ullong();
			unsigned long loop = rotationNode.attribute("loopEvery").as_ullong();

			Float scaleFactor = AS_FLOAT(rotationNode.attribute("scaleFactor"));

			std::vector<Float> speedVals = stringToSpeedVals(rotationNode.attribute("speedParams").as_string());

			stage->m_animations.push_back(new Scale(scaleFactor, speedVals, start, end, loop, target));
		}

		static void addAnimations(const pugi::xml_node& animationsNode, const std::unordered_map<unsigned int, Vec2D>& points,
								  const std::unordered_map<id_val, Animation::Target>& indexes, 
								  Stage* stage, const bool mirrorModeOn) {

			static const std::string c_rotate = "rotate", c_translate = "translate", c_toggle = "toggle", c_scale = "scale";

			for (auto it = animationsNode.begin(); it != animationsNode.end(); it++) {
				if (it->name() == c_rotate) {
					addRotation(*it, indexes, stage, mirrorModeOn);
				}

				else if (it->name() == c_translate) {
					addTranslation(*it, points, indexes, stage, mirrorModeOn);
				}

				else if (it->name() == c_toggle) {
					addToggle(*it, indexes, stage);
				}

				else if (it->name() == c_scale) {
					addScale(*it, indexes, stage);
				}
			}
		}
};


class RoundOptions {
	public:
		typedef enum GridOptions {
			NONE = 0,
			SIMPLE,
			DETAILED
		};

		RoundOptions() {
			m_gridOption = GridOptions::NONE;
			m_mirrorMode = false;
			m_invertMovingParticleCharges = false;
		}

		RoundOptions(const GridOptions& gridOptions, const bool& mirrorMode, const bool& invertMovingParticleCharges) {
			m_gridOption = gridOptions;
			m_mirrorMode = mirrorMode;
			m_invertMovingParticleCharges = invertMovingParticleCharges;
		}

		~RoundOptions() {}

		GridOptions getGridOption() const {
			return m_gridOption;
		}

		bool isMirrorModeOn() const {
			return m_mirrorMode;
		}

		bool invertMovingParticleCharges() const {
			return m_invertMovingParticleCharges;
		}


		static RoundOptions defaultOptions() {
			RoundOptions options(GridOptions::NONE, false, false);
			return options;
		}

	private:
		GridOptions m_gridOption;
		bool m_mirrorMode;
		bool m_invertMovingParticleCharges;
};

class Round {
	public:
		typedef enum GridOptions {
			NONE = 0,
			SIMPLE,
			DETAILED
		};

		typedef enum State {
			PLACING_PARTICLES = 0,
			PLAYING,
			SUCCESS,
			FAILURE
		};

		Round(const std::string stageName = "", const Round::State& initialState = PLACING_PARTICLES, 
			  const RoundOptions& roundOptions = RoundOptions::defaultOptions()) :
			m_toBePlacedStaticParticle(0, 0, 1) {

			m_stage = LoadStageXML::loadStage(stageName, roundOptions.isMirrorModeOn(), roundOptions.invertMovingParticleCharges());
			m_updateCounter = 0;
			m_roundState = initialState;
			m_initialRoundState = initialState;
			m_gridOption = roundOptions.getGridOption();
		}

		~Round() {
			delete m_stage;
			m_stage = nullptr;
		}


		void update(const DrawOptions& drawOptions) {
			switch (m_roundState) {
				case PLACING_PARTICLES:
					updatePlacingParticles(drawOptions);
					break;

				case PLAYING:
					updatePlaying();
					break;

				case SUCCESS:
				case FAILURE:
					updateEnd();

				default:
					break;
			}
		}

		void reset() {
			m_stage->reset();
			m_updateCounter = 0;
			m_roundState = m_initialRoundState;
		}


		void setStaticParticles(const std::list<StaticParticle>& staticParticles) {
			for (auto it = staticParticles.cbegin(); it != staticParticles.cend(); it++) {
				m_stage->addStaticParticle(*it);
			}
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) {
			static const Vec2D d_virtualTextPosition(50, 50);
			static const Vec2D d_virtualTimeTextPosition(50, 110);

			Vec2D realPosition = drawOptions.transform(d_virtualTextPosition);
			int realFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_TITLE);


			Vec2D realTimeTextPosition = drawOptions.transform(d_virtualTimeTextPosition);
			int realTimeTextSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_NORMAL);

			Float timeTaken = m_updateCounter / (Float)(GAME_UPDATES_PER_FRAME * FPS);
			std::string timeText = "Time: " + std::to_string(timeTaken);
			timeText = timeText.substr(0, timeText.length() - 3) + " secs.";

			Colour simpleGridColour = colourPalette.colours.at("grid");
			Colour detailedGridColour = simpleGridColour;
			detailedGridColour.a /= 2;

			switch (m_gridOption) {
				case GridOptions::DETAILED:
					for (int x = 0; x <= VIRTUAL_SCREEN_WIDTH; x += 40) {
						if (x % 200 != 0) {
							Vec2D point1(x, 0), point2(x, VIRTUAL_SCREEN_HEIGHT);
							point1 = drawOptions.transform(point1);
							point2 = drawOptions.transform(point2);
							Draw::line(detailedGridColour, point1, point2);
						}
					}
					for (int y = 0; y <= VIRTUAL_SCREEN_HEIGHT; y += 40) {
						if (y % 200 != 0) {
							Vec2D point1(0, y), point2(VIRTUAL_SCREEN_WIDTH, y);
							point1 = drawOptions.transform(point1);
							point2 = drawOptions.transform(point2);
							Draw::line(detailedGridColour, point1, point2);
						}
					}

				case GridOptions::SIMPLE:
					for (int x = 0; x <= VIRTUAL_SCREEN_WIDTH; x += 200) {
						Vec2D point1(x, 0), point2(x, VIRTUAL_SCREEN_HEIGHT);
						point1 = drawOptions.transform(point1);
						point2 = drawOptions.transform(point2);
						Draw::line(simpleGridColour, point1, point2);
					}
					for (int y = 0; y <= VIRTUAL_SCREEN_HEIGHT; y += 200) {
						Vec2D point1(0, y), point2(VIRTUAL_SCREEN_WIDTH, y);
						point1 = drawOptions.transform(point1);
						point2 = drawOptions.transform(point2);
						Draw::line(simpleGridColour, point1, point2);
					}

				case GridOptions::NONE:
					break;

				default:
					break;
			}

			m_stage->draw(drawOptions, colourPalette);

			switch (m_roundState) {
				case Round::PLACING_PARTICLES:
					m_toBePlacedStaticParticle.draw(drawOptions, colourPalette);
					break;

				case Round::SUCCESS:
					Draw::text(colourPalette.colours.at("successText"), realPosition, realFontSize, STAGE_SUCCESS_MESSAGE);
					Draw::text(colourPalette.colours.at("normalText"), realTimeTextPosition, realTimeTextSize, timeText);
					break;

				case Round::FAILURE:
					Draw::text(colourPalette.colours.at("failureText"), realPosition, realFontSize, STAGE_FAILURE_MESSAGE);
					break;

				default:
					break;
			}
		}

	private:
		Stage* m_stage;
		unsigned long m_updateCounter;
		Round::State m_roundState, m_initialRoundState;
		StaticParticle m_toBePlacedStaticParticle;
		RoundOptions::GridOptions m_gridOption;


		void updatePlacingParticles(const DrawOptions& drawOptions) {
			m_toBePlacedStaticParticle.setPosition(Input::getMousePosition(drawOptions));

			if (Input::isKeyReleased('P')) {
				m_roundState = Round::PLAYING;
			}

			else if (Input::isKeyReleased('O')) {
				m_toBePlacedStaticParticle.invertCharge();
			}

			else if (Input::isMouseReleased('L')) {
				m_stage->addStaticParticle(m_toBePlacedStaticParticle);
			}

			else if (Input::isMouseReleased('R')) {
				m_stage->removeStaticParticles(Input::getMousePosition(drawOptions));
			}

			else if (Input::isKeyReleased('R')) {
				m_stage->storeReplay();
			}

			else if (Input::itemsBeenDropped()) {
				m_stage->loadReplay();
			}
		}

		void updatePlaying() {
			m_stage->update(m_updateCounter);

			if (Input::isKeyReleased('P')) {
				reset();
			}

			else if (m_stage->anyMovingParticles()) {
				if (m_stage->allMovingParticlesTouchingGoals()) {
					m_roundState = Round::SUCCESS;
				}

				else if (m_stage->isMovingParticleColliding()) {
					m_roundState = Round::FAILURE;
				}
			}
		}

		void updateEnd() {
			if (Input::isKeyReleased('P')) {
				reset();
				m_roundState = Round::PLACING_PARTICLES;
			}

			else if (Input::isKeyReleased('R')) {
				m_stage->storeReplay();
			}
		}
};


class LevelSelect {
	public:
		LevelSelect() {
			Vec2D levelSelectButtonSize(220, 30, Vec2D::CARTESIAN);
			for (size_t w = 0; w < s_levelSelectGridWidth; w++) {
				for (size_t h = 0; h < s_levelSelectGridHeight; h++) {
					Vec2D levelSelectButtonPosition(40 + (250 * w), 220 + (40 * h), Vec2D::CARTESIAN);
					m_levelSelectGrid[h][w].set(levelSelectButtonPosition, levelSelectButtonSize, "", TEXT_FONT_SIZE_NORMAL, false);
				}
			}

			m_maxPages = ceil(Input::getItemsInDirectory(OFFICIAL_STAGES_DIR) / (Float)s_nLevelSelectButtons);
			m_page = 1;

			m_officialStagesShown = true;
			loadStageNames(OFFICIAL_STAGES_DIR, 1);

			// Back to main menu button
			static const Vec2D d_backButtonPos(40, 180), d_backButtonSize(60, 30);
			m_levelSelectAuxButtons[0].set(d_backButtonPos, d_backButtonSize, "Back", TEXT_FONT_SIZE_NORMAL, false);

			// Official/custom levels buttons
			static const Vec2D d_officialLevelPos(120, 180), d_customLevelPos(450, 180), d_levelSize(310, 30);
			bool customStagesFolderEmpty = (Input::getItemsInDirectory(CUSTOM_STAGES_DIR) == 0);
			m_levelSelectAuxButtons[1].set(d_officialLevelPos, d_levelSize, "Official levels", TEXT_FONT_SIZE_NORMAL, true);
			m_levelSelectAuxButtons[2].set(d_customLevelPos, d_levelSize, "Custom levels", TEXT_FONT_SIZE_NORMAL, customStagesFolderEmpty);
				
			// Begin/end levels buttons
			static const Vec2D d_beginButtonPos(40, 540), d_endButtonPos(680, 540), d_beginEndButtonSize(80, 30);
			m_levelSelectAuxButtons[3].set(d_beginButtonPos, d_beginEndButtonSize, "Begin", TEXT_FONT_SIZE_NORMAL, false);
			m_levelSelectAuxButtons[4].set(d_endButtonPos, d_beginEndButtonSize, "End", TEXT_FONT_SIZE_NORMAL, false);

			// Jump 10 pages forwards/backwards buttons
			static const Vec2D d_forward10ButtonPos(140, 540), d_backward10ButtonPos(600, 540), d_skip10ButtonsSize(60, 30);
			m_levelSelectAuxButtons[5].set(d_forward10ButtonPos, d_skip10ButtonsSize, "<< 10", TEXT_FONT_SIZE_NORMAL, false);
			m_levelSelectAuxButtons[6].set(d_backward10ButtonPos, d_skip10ButtonsSize, ">> 10", TEXT_FONT_SIZE_NORMAL, false);

			// Jump 1 page forwards/backwards buttons
			static const Vec2D d_forward1ButtonPos(220, 540), d_backward1ButtonPos(520, 540), d_skip1ButtonsSize(60, 30);
			m_levelSelectAuxButtons[7].set(d_forward1ButtonPos, d_skip1ButtonsSize, "<< 1", TEXT_FONT_SIZE_NORMAL, false);
			m_levelSelectAuxButtons[8].set(d_backward1ButtonPos, d_skip1ButtonsSize, ">> 1", TEXT_FONT_SIZE_NORMAL, false);
		}


		std::string isBackButtonReleased(const char& mouseButton, const DrawOptions& drawOptions) const {
			return m_levelSelectAuxButtons[0].isReleased(mouseButton, drawOptions);
		}

		std::string isAnyLevelReleased(const char& mouseButton, const DrawOptions& drawOptions) const {
			for (size_t w = 0; w < s_levelSelectGridWidth; w++) {
				for (size_t h = 0; h < s_levelSelectGridHeight; h++) {
					if (m_levelSelectGrid[h][w].isReleased(mouseButton, drawOptions) != "\n") {
						if (m_levelSelectGrid[h][w].isReleased(mouseButton, drawOptions) != "") {
							std::string returnString = (m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR);
							return returnString + "/" + m_levelSelectGrid[h][w].isReleased(mouseButton, drawOptions);
						}
						else {
							return "\n";
						}
					}
				}
			}

			return "\n";
		}


		void update(const DrawOptions& drawOptions) {
			if (m_levelSelectAuxButtons[1].isReleased('L', drawOptions) != "\n") { // shows first page of official levels
				m_maxPages = ceil(Input::getItemsInDirectory(OFFICIAL_STAGES_DIR) / (Float)s_nLevelSelectButtons);
				m_page = 1;

				m_officialStagesShown = true;
				loadStageNames(OFFICIAL_STAGES_DIR, 1);

				m_levelSelectAuxButtons[1].setDisabled(true);
				m_levelSelectAuxButtons[2].setDisabled(false);
			}

			else if (m_levelSelectAuxButtons[2].isReleased('L', drawOptions) != "\n") { // shows first page of custom levels
				m_maxPages = ceil(Input::getItemsInDirectory(CUSTOM_STAGES_DIR) / (Float)s_nLevelSelectButtons);
				m_page = 1;

				m_officialStagesShown = false;
				loadStageNames(CUSTOM_STAGES_DIR, 1);

				m_levelSelectAuxButtons[2].setDisabled(true);
				m_levelSelectAuxButtons[1].setDisabled(false);
			}

			else if (m_levelSelectAuxButtons[3].isReleased('L', drawOptions) != "\n") { // Goes to first page
				m_page = 1;

				loadStageNames((m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR), m_page);
			}

			else if (m_levelSelectAuxButtons[4].isReleased('L', drawOptions) != "\n") { // Goes to last page
				m_page = m_maxPages;

				loadStageNames((m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR), m_page);
			}

			else if (m_levelSelectAuxButtons[5].isReleased('L', drawOptions) != "\n") { // Goes back 10 pages or to first page if not possible
				m_page = ((m_page > 10) ? m_page - 10 : 1);

				loadStageNames((m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR), m_page);
			}

			else if (m_levelSelectAuxButtons[6].isReleased('L', drawOptions) != "\n") { // Goes forward 10 pages or to last page if not possible
				m_page = std::min(m_page + 10, m_maxPages);

				loadStageNames((m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR), m_page);
			}

			else if (m_levelSelectAuxButtons[7].isReleased('L', drawOptions) != "\n") { // Goes back 1 page or stays if not possible
				m_page = ((m_page > 1) ? m_page - 1 : 1);

				loadStageNames((m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR), m_page);
			}

			else if (m_levelSelectAuxButtons[8].isReleased('L', drawOptions) != "\n") { // Goes forward 1 page or stays if not possible
				m_page = std::min(m_page + 1, m_maxPages);

				loadStageNames((m_officialStagesShown ? OFFICIAL_STAGES_DIR : CUSTOM_STAGES_DIR), m_page);
			}
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			static const Vec2D d_virtualBackgroundPos(20, 160), d_virtualBackgroundSize(760, 420);
			Vec2D realBackgroundPos = drawOptions.transform(d_virtualBackgroundPos);
			Vec2D realBackgroundSize = drawOptions.scaleOnly(d_virtualBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("menuBackground"), realBackgroundPos, realBackgroundSize);

			for (size_t w = 0; w < s_levelSelectGridWidth; w++) {
				for (size_t h = 0; h < s_levelSelectGridHeight; h++) {
					Vec2D levelSelectButtonPosition(40 + (380 * w), 100 + (40 * h), Vec2D::CARTESIAN);
					m_levelSelectGrid[h][w].draw(drawOptions, colourPalette);
				}
			}

			for (size_t i = 0; i < s_nLevelSelectAuxButtons; i++) {
				m_levelSelectAuxButtons[i].draw(drawOptions, colourPalette);
			}

			std::string pageText = "Page " + std::to_string(m_page) + "/" + std::to_string(m_maxPages);
			Vec2D virtualPageTextPos(300, 545, Vec2D::CARTESIAN);
			Vec2D realPageTextPos = drawOptions.transform(virtualPageTextPos);
			int realPageTextSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_NORMAL);
			Draw::text(colourPalette.colours.at("normalText"), realPageTextPos, realPageTextSize, pageText);
		}

	private:
		static const size_t s_nLevelSelectAuxButtons = 9;
		Button m_levelSelectAuxButtons[s_nLevelSelectAuxButtons]; // Check default constructor for what each button does
		static const size_t s_levelSelectGridHeight = 8;
		static const size_t s_levelSelectGridWidth = 3;
		static const size_t s_nLevelSelectButtons = s_levelSelectGridHeight * s_levelSelectGridWidth;
		Button m_levelSelectGrid[s_levelSelectGridHeight][s_levelSelectGridWidth];
		size_t m_page, m_maxPages;
		bool m_officialStagesShown;

		void loadStageNames(const std::string& directory, const size_t& page) {
			std::vector<std::string> levelNames = Input::loadFileNames(directory, (page - 1) * s_nLevelSelectButtons, page * s_nLevelSelectButtons);

			for (size_t w = 0; w < s_levelSelectGridWidth; w++) {
				for (size_t h = 0; h < s_levelSelectGridHeight; h++) {
					Vec2D levelSelectButtonPosition(40 + (250 * w), 220 + (40 * h), Vec2D::CARTESIAN);
					m_levelSelectGrid[h][w].setText(levelNames[(w * s_levelSelectGridHeight) + h]);
				}
			}
		}
};


class TutorialWindow {
	public:
		TutorialWindow(std::string tutorialName) : 
			m_round(tutorialName, Round::PLAYING) {
				
			LoadStageXML::TutorialReturn data = LoadStageXML::loadTutorialSpecificData(tutorialName);

			m_title = data.title;
			m_text = data.text;
			m_loopTime = data.loopTime;
			m_currentTime = 0;

			m_round.setStaticParticles(data.staticParticles);

			static const Vec2D d_backButtonPos(40, 530), d_backButtonSize(720, 30);
			m_backButton.set(d_backButtonPos, d_backButtonSize, "Got it", TEXT_FONT_SIZE_NORMAL, false);
		}

		~TutorialWindow() {}


		std::string isBackButtonReleased(const char& mouseButton, const DrawOptions& drawOptions) const {
			return m_backButton.isReleased(mouseButton, drawOptions);
		}


		void update(const DrawOptions& drawOptions) {
			m_round.update(drawOptions);
			m_currentTime += GAME_UPDATES_PER_FRAME;
			if (m_currentTime > m_loopTime) {
				m_round.reset();
				m_currentTime = 0;
			}
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) {
			static const Vec2D d_virtualBackgroundPos(20, 160), d_virtualBackgroundSize(760, 420);
			Vec2D realBackgroundPos = drawOptions.transform(d_virtualBackgroundPos);
			Vec2D realBackgroundSize = drawOptions.scaleOnly(d_virtualBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("menuBackground"), realBackgroundPos, realBackgroundSize);

			m_backButton.draw(drawOptions, colourPalette);

			static const Vec2D d_virtualRoundPos(40, 195), d_virtualRoundSize(400, 300);
			Vec2D realRoundPos = drawOptions.transform(d_virtualRoundPos);
			Vec2D realRoundSize = drawOptions.scaleOnly(d_virtualRoundSize);
			Draw::rectangle(colourPalette.colours.at("background"), realRoundPos, realRoundSize);

			DrawOptions virtualRoundDrawOptions(d_virtualRoundPos * 2, 0.5);
			DrawOptions realRoundDrawOptions(drawOptions, virtualRoundDrawOptions);
			m_round.draw(realRoundDrawOptions, colourPalette);


			static const Vec2D d_virtualTextboxPos(460, 180), d_virtualTextboxSize(300, 330);
			Vec2D realTextboxPos = drawOptions.transform(d_virtualTextboxPos);
			Vec2D realTextboxSize = drawOptions.scaleOnly(d_virtualTextboxSize);
			Draw::rectangle(colourPalette.colours.at("background"), realTextboxPos, realTextboxSize);

			static const Vec2D d_virtualTitlePos(468, 190);
			Vec2D realTitlePos = drawOptions.transform(d_virtualTitlePos);
			Float realTitleFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_SUBTITLE);
			Draw::text(colourPalette.colours.at("normalText"), realTitlePos, realTitleFontSize, m_title);

			Float realLineFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_NORMAL);
			for (size_t i = 0; i < m_text.size(); i++) {
				Vec2D virtualLinePos(468, 240 + (22 * i));
				Vec2D realLinePos = drawOptions.transform(virtualLinePos);

				Draw::text(colourPalette.colours.at("normalText"), realLinePos, realLineFontSize, m_text[i]);
			}
		}

	private:
		Round m_round;
		std::string m_title;
		std::vector<std::string> m_text;
		Button m_backButton;
		unsigned long m_loopTime, m_currentTime;
};

class TutorialsMenu {
	public:
		TutorialsMenu() {
			Vec2D levelSelectButtonSize(345, 30, Vec2D::CARTESIAN);
			for (size_t w = 0; w < s_tutorialsGridWidth; w++) {
				for (size_t h = 0; h < s_tutorialsGridHeight; h++) {
					Vec2D levelSelectButtonPosition(40 + (375 * w), 220 + (40 * h), Vec2D::CARTESIAN);
					m_tutorialSelectGrid[h][w].set(levelSelectButtonPosition, levelSelectButtonSize, "", TEXT_FONT_SIZE_NORMAL, false);
				}
			}

			loadStageNames(TUTORIALS_DIR);

			static const Vec2D d_backButtonPos(40, 180), d_backButtonSize(60, 30);
			m_backButton.set(d_backButtonPos, d_backButtonSize, "Back", TEXT_FONT_SIZE_NORMAL, false);
		}


		std::string isBackButtonReleased(const char& mouseButton, const DrawOptions& drawOptions) const {
			return m_backButton.isReleased(mouseButton, drawOptions);
		}

		std::string isTutorialSelected(const char& mouseButton, const DrawOptions& drawOptions) const {
			for (size_t w = 0; w < s_tutorialsGridWidth; w++) {
				for (size_t h = 0; h < s_tutorialsGridHeight; h++) {
					std::string releasedResult = m_tutorialSelectGrid[h][w].isReleased(mouseButton, drawOptions);
					if (releasedResult != "\n") {
						return ((releasedResult == "") ? "\n" : releasedResult);
					}
				}
			}

			return "\n";
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			static const Vec2D d_virtualBackgroundPos(20, 160), d_virtualBackgroundSize(760, 420);
			Vec2D realBackgroundPos = drawOptions.transform(d_virtualBackgroundPos);
			Vec2D realBackgroundSize = drawOptions.scaleOnly(d_virtualBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("menuBackground"), realBackgroundPos, realBackgroundSize);

			for (size_t w = 0; w < s_tutorialsGridWidth; w++) {
				for (size_t h = 0; h < s_tutorialsGridHeight; h++) {
					Vec2D levelSelectButtonPosition(40 + (380 * w), 100 + (40 * h), Vec2D::CARTESIAN);
					m_tutorialSelectGrid[h][w].draw(drawOptions, colourPalette);
				}
			}

			m_backButton.draw(drawOptions, colourPalette);
		}

	private:
		Button m_backButton;

		static const size_t s_tutorialsGridHeight = 9;
		static const size_t s_tutorialsGridWidth = 2;
		static const size_t s_nTutorialsButtons = s_tutorialsGridHeight * s_tutorialsGridWidth;
		Button m_tutorialSelectGrid[s_tutorialsGridHeight][s_tutorialsGridWidth];

		void loadStageNames(const std::string& directory) {
			std::vector<std::string> tutorialsNames = Input::loadFileNames(directory, 0, s_nTutorialsButtons);

			for (size_t w = 0; w < s_tutorialsGridWidth; w++) {
				for (size_t h = 0; h < s_tutorialsGridHeight; h++) {
					std::string text = tutorialsNames[(w * s_tutorialsGridHeight) + h];
					text = text.substr(0, text.size() - XML_FILE_EXTENSION.size());
					for (size_t i = 0; i < text.size(); i++) {
						text[i] = ((text[i] == '_') ? ' ' : text[i]);
					}

					Vec2D levelSelectButtonPosition(40 + (250 * w), 220 + (40 * h), Vec2D::CARTESIAN);
					m_tutorialSelectGrid[h][w].setText(text);
				}
			}
		}
};


class OptionsMenu {
	public:
		OptionsMenu() {
			static const Vec2D d_virtualOptionsButtonSize(220, 30);

			static const Vec2D d_virtualGridOption1Pos(55, 222), d_virtualGridOption2Pos(290, 222), d_virtualGridOption3Pos(525, 222);
			m_gridOptions[0].set(d_virtualGridOption1Pos, d_virtualOptionsButtonSize, "None", TEXT_FONT_SIZE_NORMAL, true);
			m_gridOptions[1].set(d_virtualGridOption2Pos, d_virtualOptionsButtonSize, "Simple", TEXT_FONT_SIZE_NORMAL, false);
			m_gridOptions[2].set(d_virtualGridOption3Pos, d_virtualOptionsButtonSize, "Detailed", TEXT_FONT_SIZE_NORMAL, false);

			static const Vec2D d_virtualMirrorModeActivatePos(155, 322), d_virtualMirrorModeDeactivatePos(425, 322);
			m_mirrorMode[0].set(d_virtualMirrorModeActivatePos, d_virtualOptionsButtonSize, "Activated", TEXT_FONT_SIZE_NORMAL, false);
			m_mirrorMode[1].set(d_virtualMirrorModeDeactivatePos, d_virtualOptionsButtonSize, "Deactivated", TEXT_FONT_SIZE_NORMAL, true);

			static const Vec2D d_virtualMovingParticleChargeNormalPos(155, 422), d_virtualMovingParticleChargeInvertedPos(425, 422);
			m_movingParticleCharge[0].set(d_virtualMovingParticleChargeNormalPos, d_virtualOptionsButtonSize, 
										  "Normal", TEXT_FONT_SIZE_NORMAL, true);
			m_movingParticleCharge[1].set(d_virtualMovingParticleChargeInvertedPos, d_virtualOptionsButtonSize, 
										  "Inverted", TEXT_FONT_SIZE_NORMAL, false);

			static const Vec2D d_backButtonPos(40, 510), d_backButtonSize(500, 50);
			m_backButton.set(d_backButtonPos, d_backButtonSize, "Save and go back", TEXT_FONT_SIZE_NORMAL, false);
		}

		~OptionsMenu() {}

		void update(const DrawOptions& drawOptions) {
			for (size_t i = 0; i < 3; i++) {
				if (m_gridOptions[i].isReleased('L', drawOptions) != "\n") {
					for (size_t j = 0; j < 3; j++) {
						m_gridOptions[j].setDisabled(i == j);
					}
				}
			}

			for (size_t i = 0; i < 2; i++) {
				if (m_mirrorMode[i].isReleased('L', drawOptions) != "\n") {
					for (size_t j = 0; j < 2; j++) {
						m_mirrorMode[j].setDisabled(i == j);
					}
				}
			}

			for (size_t i = 0; i < 2; i++) {
				if (m_movingParticleCharge[i].isReleased('L', drawOptions) != "\n") {
					for (size_t j = 0; j < 2; j++) {
						m_movingParticleCharge[j].setDisabled(i == j);
					}
				}
			}
		}


		std::string isBackButtonReleased(const char& mouseButton, const DrawOptions& drawOptions) const {
			return m_backButton.isReleased(mouseButton, drawOptions);
		}

		RoundOptions getOptions() const {
			RoundOptions::GridOptions gridOption = getGridOption();
			bool mirrorMode = m_mirrorMode[0].isDisabled();
			bool invertMovingParticleCharges = m_movingParticleCharge[1].isDisabled();

			RoundOptions options(gridOption, mirrorMode, invertMovingParticleCharges);
			return options;
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) const {
			static const Vec2D d_virtualBackgroundPos(20, 160), d_virtualBackgroundSize(760, 420);
			Vec2D realBackgroundPos = drawOptions.transform(d_virtualBackgroundPos);
			Vec2D realBackgroundSize = drawOptions.scaleOnly(d_virtualBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("menuBackground"), realBackgroundPos, realBackgroundSize);

			static const Vec2D d_virtualGridOptionsBackgroundPos(40, 180), d_virtualGridOptionsBackgroundSize(720, 80);
			Vec2D realGridOptionsBackgroundPos = drawOptions.transform(d_virtualGridOptionsBackgroundPos);
			Vec2D realGridOptionsBackgroundSize = drawOptions.scaleOnly(d_virtualGridOptionsBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("background"), realGridOptionsBackgroundPos, realGridOptionsBackgroundSize);
				

			static const int d_headerFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_SUBTITLE);
			static const int d_selectedOptionFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_NORMAL);

			drawGridOptions(drawOptions, colourPalette, d_headerFontSize, d_selectedOptionFontSize);
			drawMirrorMode(drawOptions, colourPalette, d_headerFontSize, d_selectedOptionFontSize);
			drawMovingParticleCharge(drawOptions, colourPalette, d_headerFontSize, d_selectedOptionFontSize);

			m_backButton.draw(drawOptions, colourPalette);
		}

	private:
		Button m_backButton;

		Button m_gridOptions[3];
		Button m_mirrorMode[2];
		Button m_movingParticleCharge[2];


		void drawGridOptions(const DrawOptions& drawOptions, const ColourPalette& colourPalette, 
							 const int& headerFontSize, const int& selectedOptionFontSize) const {

			static const Vec2D d_virtualGridOptionsBackgroundPos(40, 180), d_virtualGridOptionsBackgroundSize(720, 80);
			Vec2D realGridOptionsBackgroundPos = drawOptions.transform(d_virtualGridOptionsBackgroundPos);
			Vec2D realGridOptionsBackgroundSize = drawOptions.scaleOnly(d_virtualGridOptionsBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("background"), realGridOptionsBackgroundPos, realGridOptionsBackgroundSize);

			static const Vec2D d_virtualGridOptionsHeaderPosition(55, 188);
			Vec2D realGridOptionsHeaderPosition = drawOptions.transform(d_virtualGridOptionsHeaderPosition);
			Draw::text(colourPalette.colours.at("normalText"), realGridOptionsHeaderPosition, headerFontSize, "Grid:");

			std::string optionSelected = "";
			for (size_t i = 0; i < 3; i++) {
				m_gridOptions[i].draw(drawOptions, colourPalette);
					
				if (m_gridOptions[i].isDisabled()) {
					optionSelected = m_gridOptions[i].getText();
				}
			}

			static const Vec2D d_virtualGridOptionsSelectedOptionPosition(129, 188 + ((headerFontSize - selectedOptionFontSize) / 2));
			Vec2D realGridOptionsSelectedOptionPosition = drawOptions.transform(d_virtualGridOptionsSelectedOptionPosition);
			Draw::text(colourPalette.colours.at("normalText"), realGridOptionsSelectedOptionPosition, selectedOptionFontSize, optionSelected);
		}

		void drawMirrorMode(const DrawOptions& drawOptions, const ColourPalette& colourPalette,
							const int& headerFontSize, const int& selectedOptionFontSize) const {

			static const Vec2D d_virtualMirrorModeBackgroundPos(40, 280), d_virtualMirrorModeBackgroundSize(720, 80);
			Vec2D realMirrorModeBackgroundPos = drawOptions.transform(d_virtualMirrorModeBackgroundPos);
			Vec2D realMirrorModeBackgroundSize = drawOptions.scaleOnly(d_virtualMirrorModeBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("background"), realMirrorModeBackgroundPos, realMirrorModeBackgroundSize);

			static const Vec2D d_virtualMirrorModeHeaderPosition(55, 288);
			Vec2D realMirrorModeHeaderPosition = drawOptions.transform(d_virtualMirrorModeHeaderPosition);
			Draw::text(colourPalette.colours.at("normalText"), realMirrorModeHeaderPosition, headerFontSize, "Mirror mode:");

			std::string optionSelected = "";
			for (size_t i = 0; i < 2; i++) {
				m_mirrorMode[i].draw(drawOptions, colourPalette);

				if (m_mirrorMode[i].isDisabled()) {
					optionSelected = m_mirrorMode[i].getText();
				}
			}

			static const Vec2D d_virtualMirrorModeActivatedTextPosition(250, 288 + ((headerFontSize - selectedOptionFontSize) / 2));
			Vec2D realMirrorModeActivatedTextPosition = drawOptions.transform(d_virtualMirrorModeActivatedTextPosition);
			Draw::text(colourPalette.colours.at("normalText"), realMirrorModeActivatedTextPosition, selectedOptionFontSize, optionSelected);
		}

		void drawMovingParticleCharge(const DrawOptions& drawOptions, const ColourPalette& colourPalette,
									  const int& headerFontSize, const int& selectedOptionFontSize) const {

			static const Vec2D d_virtualMovingParticleChargeBackgroundPos(40, 380), d_virtualMovingParticleChargeBackgroundSize(720, 80);
			Vec2D realMovingParticleChargeBackgroundPos = drawOptions.transform(d_virtualMovingParticleChargeBackgroundPos);
			Vec2D realMovingParticleChargeBackgroundSize = drawOptions.scaleOnly(d_virtualMovingParticleChargeBackgroundSize);
			Draw::rectangle(colourPalette.colours.at("background"), realMovingParticleChargeBackgroundPos, 
							realMovingParticleChargeBackgroundSize);

			static const Vec2D d_virtualMovingParticleChargeHeaderPosition(55, 388);
			Vec2D realMovingParticleChargeHeaderPosition = drawOptions.transform(d_virtualMovingParticleChargeHeaderPosition);
			Draw::text(colourPalette.colours.at("normalText"), realMovingParticleChargeHeaderPosition, headerFontSize, "Particle charge:");

			std::string optionSelected = "";
			for (size_t i = 0; i < 2; i++) {
				m_movingParticleCharge[i].draw(drawOptions, colourPalette);

				if (m_movingParticleCharge[i].isDisabled()) {
					optionSelected = m_movingParticleCharge[i].getText();
				}
			}

			static const Vec2D d_virtualMovingParticleChargeTextPosition(305, 388 + ((headerFontSize - selectedOptionFontSize) / 2));
			Vec2D realMovingParticleChargeTextPosition = drawOptions.transform(d_virtualMovingParticleChargeTextPosition);
			Draw::text(colourPalette.colours.at("normalText"), realMovingParticleChargeTextPosition, selectedOptionFontSize, optionSelected);
		}

		RoundOptions::GridOptions getGridOption() const {
			if (m_gridOptions[1].isDisabled()) {
				return RoundOptions::GridOptions::SIMPLE;
			}
				
			else if (m_gridOptions[2].isDisabled()) {
				return RoundOptions::GridOptions::DETAILED;
			}

			return RoundOptions::GridOptions::NONE;
		}
};


class MainMenu {
	public:
		typedef enum State {
			TITLE = 0,
			LEVEL_SELECT,
			TUTORIAL_SELECT,
			TUTORIAL_SHOWN,
			OPTIONS
		};

		MainMenu() {
			m_previousMainMenuState = MainMenu::TITLE;
			m_mainMenuState = MainMenu::TITLE;

			// Opens level select menu
			static const Vec2D d_levelSelectButtonPos(50, 450), d_levelSelectButtonSize(700, 50);
			m_mainButtons[0].set(d_levelSelectButtonPos, d_levelSelectButtonSize, "Select level", TEXT_FONT_SIZE_NORMAL, false);
			m_mainButtonsInitiallyEnabled[0] = false;

			// Opens options menu
			static const Vec2D d_optionsButtonPos(412.5, 525), d_optionsButtonSize(337.5, 50);
			m_mainButtons[1].set(d_optionsButtonPos, d_optionsButtonSize, "Options", TEXT_FONT_SIZE_NORMAL, false);
			m_mainButtonsInitiallyEnabled[1] = false;

			// Opens tutorial menu
			static const Vec2D d_tutorialsButtonPos(50, 525), d_tutorialsButtonSize(337.5, 50);
			m_mainButtons[2].set(d_tutorialsButtonPos, d_tutorialsButtonSize, "Tutorials", TEXT_FONT_SIZE_NORMAL, false);
			m_mainButtonsInitiallyEnabled[2] = false;

			m_tutorial = nullptr;

			m_noticeText = nullptr;
		}

		~MainMenu() {
			if (m_tutorial != nullptr) {
				delete m_tutorial;
			}
		}


		void update(const DrawOptions& drawOptions) {
			m_previousMainMenuState = m_mainMenuState;

			if (m_mainMenuState == MainMenu::TITLE) {
				bool stateHasChanged = false;

				if (m_mainButtons[0].isReleased('L', drawOptions) != "\n") {
					m_mainMenuState = MainMenu::LEVEL_SELECT;
					stateHasChanged = true;
				}

				else if (m_mainButtons[1].isReleased('L', drawOptions) != "\n") {
					m_mainMenuState = MainMenu::OPTIONS;
					stateHasChanged = true;
				}

				else if (m_mainButtons[2].isReleased('L', drawOptions) != "\n") {
					m_mainMenuState = MainMenu::TUTORIAL_SELECT;
					stateHasChanged = true;
				}

				if (stateHasChanged) {
					for (size_t i = 0; i < s_nMainButtons; i++) {
						m_mainButtons[i].setDisabled(true);
					}
				}
			}

			else if (m_mainMenuState == MainMenu::LEVEL_SELECT) {
				m_levelSelect.update(drawOptions);

				if (m_levelSelect.isBackButtonReleased('L', drawOptions) == "Back") {
					reset();
				}
			}

			else if (m_mainMenuState == MainMenu::TUTORIAL_SELECT) {
				if (m_tutorialsMenu.isBackButtonReleased('L', drawOptions) == "Back") {
					reset();
				}

				else if (m_tutorialsMenu.isTutorialSelected('L', drawOptions) != "\n") {
					if (m_tutorial != nullptr) {
						delete m_tutorial;
					}

					std::string stageFilePath = m_tutorialsMenu.isTutorialSelected('L', drawOptions);
					for (size_t i = 0; i < stageFilePath.size(); i++) {
						stageFilePath[i] = ((stageFilePath[i] == ' ') ? '_' : stageFilePath[i]);
					}
					stageFilePath = TUTORIALS_DIR + '/' + stageFilePath + XML_FILE_EXTENSION;

					m_tutorial = new TutorialWindow(stageFilePath);

					m_mainMenuState = MainMenu::TUTORIAL_SHOWN;
				}
			}

			else if (m_mainMenuState == MainMenu::TUTORIAL_SHOWN) {
				m_tutorial->update(drawOptions);

				if (m_tutorial->isBackButtonReleased('L', drawOptions) == "Got it") {
					if (m_tutorial != nullptr) {
						delete m_tutorial;
						m_tutorial = nullptr;
					}

					m_mainMenuState = MainMenu::TUTORIAL_SELECT;
				}
			}

			else if (m_mainMenuState == MainMenu::OPTIONS) {
				m_optionsMenu.update(drawOptions);

				if (m_optionsMenu.isBackButtonReleased('L', drawOptions) != "\n") {
					reset();
				}
			}
		}

		void reset() {
			m_mainMenuState = MainMenu::TITLE;
			m_mainButtons[0].setDisabled(false);
			for (size_t i = 0; i < s_nMainButtons; i++) {
				m_mainButtons[i].setDisabled(m_mainButtonsInitiallyEnabled[i]);
			}
		}

		std::string isLevelSelected(const DrawOptions& drawOptions) const {
			return (((m_mainMenuState == MainMenu::LEVEL_SELECT) && (m_previousMainMenuState == MainMenu::LEVEL_SELECT)) ?
					m_levelSelect.isAnyLevelReleased('L', drawOptions) : "\n");
		}

		RoundOptions getOptions() const {
			return m_optionsMenu.getOptions();
		}


		void draw(const DrawOptions& drawOptions, const ColourPalette& colourPalette) {
			static const Vec2D d_virtualTitlePos(50, 50);
			Vec2D realTitlePos = drawOptions.transform(d_virtualTitlePos);
			int realTitleFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_TITLE);

			static const Vec2D d_virtualVersionPos(408, 74);
			Vec2D realVersionPos = drawOptions.transform(d_virtualVersionPos);
			int realVersionTextSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_NORMAL);

			Draw::text(colourPalette.colours.at("gameTitleText"), realTitlePos, realTitleFontSize, GAME_TITLE);
			Draw::text(colourPalette.colours.at("normalText"), realVersionPos, realVersionTextSize, GAME_VERSION);

			static const Vec2D d_virtualControlsPos(50, 120), d_virtualControlsNewLineOffset(0, 30);
			Vec2D realControlsNewLineOffset = drawOptions.scaleOnly(d_virtualControlsNewLineOffset);
			int realControlsFontSize = drawOptions.scaleOnly(TEXT_FONT_SIZE_NORMAL);

			for (size_t i = 0; i < CONTROLS_TEXT.size(); i++) {
				Vec2D realControlsPos = drawOptions.transform(d_virtualControlsPos) + (realControlsNewLineOffset * i);
				Draw::text(colourPalette.colours.at("normalText"), realControlsPos, realControlsFontSize, CONTROLS_TEXT[i]);
			}

			for (size_t i = 0; i < s_nMainButtons; i++) {
				m_mainButtons[i].draw(drawOptions, colourPalette);
			}

			switch (m_mainMenuState) {
				case MainMenu::LEVEL_SELECT:
					m_levelSelect.draw(drawOptions, colourPalette);
					break;

				case MainMenu::TUTORIAL_SELECT:
					m_tutorialsMenu.draw(drawOptions, colourPalette);
					break;

				case MainMenu::TUTORIAL_SHOWN:
					m_tutorial->draw(drawOptions, colourPalette);
					break;

				case MainMenu::OPTIONS:
					m_optionsMenu.draw(drawOptions, colourPalette);

				default:
					break;
			}
		}

	private:
		MainMenu::State m_mainMenuState, m_previousMainMenuState;

		static const size_t s_nMainButtons = 3;
		Button m_mainButtons[s_nMainButtons]; // Check default constructor for what each button does
		bool m_mainButtonsInitiallyEnabled[s_nMainButtons];

		LevelSelect m_levelSelect;

		TutorialsMenu m_tutorialsMenu;
		TutorialWindow* m_tutorial;

		OptionsMenu m_optionsMenu;

		NoticeText* m_noticeText;
};


class Game {
	public:
		typedef enum State {
			MAIN_MENU = 0,
			ROUND
		};

		Game() {
			m_round = nullptr;
			m_gameState = Game::MAIN_MENU;

			m_noticeText = nullptr;
		}

		~Game() {
			if (m_round != nullptr) {
				delete m_round;
			}

			if (m_noticeText != nullptr) {
				delete m_noticeText;
			}
		}


		void update() {
			switch (m_gameState) {
				case Game::MAIN_MENU:
					m_mainMenu.update(m_drawOptions);
						
					if (m_mainMenu.isLevelSelected(m_drawOptions) != "\n") {
						RoundOptions roundOptions = m_mainMenu.getOptions();
						m_round = new Round(m_mainMenu.isLevelSelected(m_drawOptions), Round::PLACING_PARTICLES, roundOptions);
						m_gameState = Game::ROUND;
						m_mainMenu.reset();
					}

					else if (Input::itemsBeenDropped()) {
						std::string filePath = Input::getDroppedFileLocation();
						m_colourPalette.update(filePath);

						if (m_noticeText != nullptr) {
							delete m_noticeText;
						}
						static const Vec2D d_noticeTextPos(65, 400);
						m_noticeText = new NoticeText(NoticeText::CONFIRMATION, PALETTE_SUCCESSFUL_LOAD, 
													  d_noticeTextPos, TEXT_FONT_SIZE_NORMAL, 1, 0.5);
					}

					break;

				case Game::ROUND:
					m_round->update(m_drawOptions);

					if (Input::isKeyReleased('Q')) {
						m_gameState = Game::MAIN_MENU;
						delete m_round;
						m_round = nullptr;
					}

					break;

				default:
					break;
			}
		}


		void draw() {
			Draw::background(m_colourPalette.colours.at("background"));

			switch (m_gameState) {
				case Game::MAIN_MENU:
					m_mainMenu.draw(m_drawOptions, m_colourPalette);

					if (m_noticeText != nullptr) {
						m_noticeText->draw(m_drawOptions, m_colourPalette);
					}

					break;

				case Game::ROUND:
					m_round->draw(m_drawOptions, m_colourPalette);
					break;

				default:
					break;
			}
		}

	private:
		MainMenu m_mainMenu;
		Round* m_round;

		Game::State m_gameState;

		DrawOptions m_drawOptions;
		ColourPalette m_colourPalette;

		NoticeText* m_noticeText;
};