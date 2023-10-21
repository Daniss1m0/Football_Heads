#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <iostream>
#include <cmath>
#include <string>
#include <list>

unsigned short int leftPlayerScore = 0, rightPlayerScore = 0;
bool prevGoalForLeft = false;
bool roundActive = false;

namespace phys // klasy fizyczne
{
	struct VECTOR2
	{
	public:
		float X;
		float Y;

		VECTOR2(float x = 20, float y = 20)
			: X(x), Y(y)
		{}

		VECTOR2 operator+(const VECTOR2& other)
		{
			return VECTOR2(X + other.X, Y + other.Y);
		}

		VECTOR2 Unit();

		float Magnitude();
	};

	class GATE
	{
	public:
		VECTOR2 Upper;
		VECTOR2 Lower;

		float Height;
		float Width;
		bool IsLeft = false;

		GATE(float height, float width, bool left)
			: Height(height), Width(width), IsLeft(left)
		{}
	};

	class RECTANGLE
	{
	public:
		VECTOR2 Position, Size;

		RECTANGLE(VECTOR2 position, VECTOR2 size)
			: Position(position), Size(size)
		{}
	};

	class BALL
	{
	public:
		VECTOR2 Position;
		VECTOR2 Velocity;
		std::string ShooterName;
		float Radius;
		float DefaultAcceleration;
		float Acceleration;
		bool moved;

		BALL(VECTOR2 position, VECTOR2 velocity, const std::string& shooterName, float radius, float acceleration)
			: Position(position), Velocity(velocity), ShooterName(shooterName), Radius(radius), Acceleration(acceleration)
		{}

		void Move(int szer, int wys, ALLEGRO_SAMPLE* touchSound);

		bool CollisionWithRect(const RECTANGLE& Prostokat);
		bool BounceOffPlayer(float X, float Y, float headRadius, float plrSpeedX, float plrSpeedY, ALLEGRO_SAMPLE* touchSound);
		bool CheckGoal(GATE gate, ALLEGRO_SAMPLE* goalSound);
		bool Kick(float X, float Y, ALLEGRO_SAMPLE* touchSound);

	};
}


// struktura do przechowywania graczy
class PLAYER
{
public:
	float skala = 0.75;
	float X, Y;       // pozycja zawodnika na boisku
	float SpeedX, SpeedY; // prędkość ruchu gracza
	float DefaultY = 0;
	float yAcceleration = 0;
	float HeadRadius = 50*skala;
	float HeadOffset = 20 * skala;
	float ShoeOffset = -10 * skala;
	float ShoeWidth = 200 * skala;
	float ShoeHeight = 150 * skala;
	float Width = 200.0f * skala;
	float Height = 200.0f * skala;
	bool anim_started = false; //animacja dla gracza
	bool Jumped = false; //flaga dla jump
	bool MoveLeft = false; //flaga dla sterowania
	bool DrawLeft = false; //flaga dla rysowania gracza w odpowiednim kierunku

	PLAYER(float x, float y, float speedX, float speedY)
		: X(x), Y(y), SpeedX(speedX), SpeedY(speedY)
	{}

	void Jump(float force);

	void UpdatePos(ALLEGRO_DISPLAY* display)
	{
		if (X < 0) // zapobiega wyleceniu poza mape z lewej strony
		{
			X = 0;
		}
		else if (X > al_get_display_width(display)) // zapobiega wyleceniu poza mape z prawej strony
		{
			X = al_get_display_width(display);
		}

		SpeedY += yAcceleration;
		X += SpeedX;
		Y += SpeedY;

		if (Y > DefaultY && Jumped == true)
		{
			yAcceleration = 0;
			Y = DefaultY;
			SpeedY = 0;
			Jumped = false;
		}
	}

};

void PLAYER::Jump(float force)
{
	if (Jumped == false)
	{
		yAcceleration = 0.6f;
		Jumped = true;
		SpeedY = -force;
	}

}

// startuje runde
void StartRound(PLAYER* player1, PLAYER* player2, phys::BALL* ball)
{
	if (!roundActive)
	{
		roundActive = true;

		// ustawia pozycje pilki
		if (prevGoalForLeft == false)
		{
			ball->Position.X = 400;
			ball->Position.Y = 200;
		}
		else
		{
			ball->Position.X = 800;
			ball->Position.Y = 200;
		}

		ball->Velocity.X = 0;
		ball->Velocity.Y = 0;

		// ustawia graczy na pozycjach
		player1->Jumped = false;
		player1->yAcceleration = 0;
		player1->Y = player1->DefaultY;
		player1->SpeedY = 0;
		player1->X = 300;
		player1->DrawLeft = false;

		player2->Jumped = false;
		player2->yAcceleration = 0;
		player2->Y = player2->DefaultY;
		player2->SpeedY = 0;
		player2->X = 980;
		player2->DrawLeft = false;
	}
}
bool EndRound(ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* event_queue) {
	while (true) {
		//Wyniki
		if(leftPlayerScore > rightPlayerScore)
		al_draw_text(font, al_map_rgb(0, 0, 0), 575, 50, ALLEGRO_ALIGN_CENTER, "Blue win");
		else if (rightPlayerScore > leftPlayerScore)
		al_draw_text(font, al_map_rgb(0, 0, 0), 575, 50, ALLEGRO_ALIGN_CENTER, "Green win");
		else
		al_draw_text(font, al_map_rgb(0, 0, 0), 575, 50, ALLEGRO_ALIGN_CENTER, "Draw");

		// Przycisk "Restart"
		al_draw_rectangle(520, 300, 640, 350, al_map_rgb(0, 0, 0), 2);
		al_draw_text(font, al_map_rgb(0, 0, 0), 575, 310, ALLEGRO_ALIGN_CENTER, "Restart");
		// Przycisk "Wyjście"
		al_draw_rectangle(985, 50, 1105, 100, al_map_rgb(0, 0, 0), 2);
		al_draw_text(font, al_map_rgb(0, 0, 0), 1000, 60, 0, "Wyjscie");
		al_flip_display();

		// Obsługa zdarzeń myszy
		ALLEGRO_EVENT event;
		al_wait_for_event(event_queue, &event);
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			if (event.mouse.x >= 515 && event.mouse.x <= 630 && event.mouse.y >= 300 && event.mouse.y <= 350) { //Przycisk "Restart"
				return false;
			}
			if (event.mouse.x >= 980 && event.mouse.x <= 1100 && event.mouse.y >= 50 && event.mouse.y <= 100) { //Przycisk "Wyjście"
				return true;
			}
		}		
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { //zamykanie
			return true;
		}
	}
}

bool menu(ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* event_queue) {
	while (true) {
		al_clear_to_color(al_map_rgb(255, 255, 255));
		// Przycisk "Start"
		al_draw_rectangle(520, 300, 640, 350, al_map_rgb(0, 0, 0), 2);
		al_draw_text(font, al_map_rgb(0, 0, 0), 575, 310, ALLEGRO_ALIGN_CENTER, "Start");
		// Przycisk "Wyjście"
		al_draw_rectangle(520, 400, 640, 450, al_map_rgb(0, 0, 0), 2);
		al_draw_text(font, al_map_rgb(0, 0, 0), 575, 410, ALLEGRO_ALIGN_CENTER, "Wyjscie");
		al_flip_display();

		// Obsługa zdarzeń myszy
		ALLEGRO_EVENT event;
		al_wait_for_event(event_queue, &event);
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			if (event.mouse.x >= 515 && event.mouse.x <= 630 && event.mouse.y >= 300 && event.mouse.y <= 350) { //Przycisk "Start"
				return true;
			}
			if (event.mouse.x >= 520 && event.mouse.x <= 640 && event.mouse.y >= 400 && event.mouse.y <= 450) { //Przycisk "Wyjście"
				return false;
			}
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { //zamykanie
			return false;
		}
	}
}

// metody fizyczne
namespace phys
{

	float VECTOR2::Magnitude()
	{
		return sqrt(X * X + Y * Y);
	}

	VECTOR2 VECTOR2::Unit()
	{
		VECTOR2 newValue;
		newValue.X = X / Magnitude();
		newValue.Y = Y / Magnitude();
		return newValue;
	}

	void BALL::Move(int width, int height, ALLEGRO_SAMPLE* touchSound)
	{
		// zapobiega wyleceniu poza mape:
		if (Position.Y > height - Radius)
		{
			Position.Y = height - Radius;
			if (Velocity.Y > 0)
			{
				Velocity.Y *= -0.8f;
				if (fabs(Velocity.Y) > 7)
				{
					al_stop_samples(); // stop dzwienkow
					al_play_sample(touchSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
				}
			}
		}

		if (Position.Y < 0 + Radius)
		{
			Position.Y = 0 + Radius;
			if (Velocity.Y < 0)
			{
				Velocity.Y *= -0.8f;
				if (fabs(Velocity.Y) > 7)
				{
					al_stop_samples(); // stop dzwiekow
					al_play_sample(touchSound, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
				}
			}
		}

		if (Position.X < 0 + Radius)
		{
			Position.X = Radius;
			if (Velocity.X < 0)
			{
				Velocity.X *= -0.7f;
				if (fabs(Velocity.X) > 7)
				{
					al_stop_samples(); // stop dzwiekow
					al_play_sample(touchSound, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
				}
			}
		}

		if (Position.X > width - Radius)
		{
			Position.X = width - Radius;
			if (Velocity.X > 0)
			{
				Velocity.X *= -0.7f;
				if (fabs(Velocity.X) > 7)
				{
					al_stop_samples(); // stop dzwiekow
					al_play_sample(touchSound, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
				}
			}
		}

		Velocity.Y += Acceleration;

		// ball zwalnia gdy sie toczy az w koncu staje
		if (fabs(Velocity.X) < 0.05)
		{
			Velocity.X = 0;
		}
		else
		{
			Velocity.X -= Velocity.X / fabs(Velocity.X) * 0.05; // zawsze zwalnia w przeciwnym kierunku do kierunku ruchu
		}

		Position.X += Velocity.X;
		Position.Y += Velocity.Y;
	}

	bool BALL::CollisionWithRect(const RECTANGLE& rect)
	{
		// sprawdza czy jest kolizja z prostokatem
		if (Position.X >= rect.Position.X - rect.Size.X / 2 - Radius
			&& Position.X <= rect.Position.X + rect.Size.X / 2 + Radius
			&& Position.Y >= rect.Position.Y - rect.Size.Y / 2 - Radius
			&& Position.Y <= rect.Position.Y + rect.Size.Y / 2 + Radius)
		{  // co sie dzieje gdy ball wleci w prostokat :
			std::cout << "wlecialo w prostokat" << std::endl;


			float addVel = 0;
			if (rect.Position.X > 500)
			{
				addVel = -1;
			}else
			{
				addVel = -1;
			}

			if (Position.X < rect.Position.X - rect.Size.X / 2) // tutaj sprawdza po srodku pilki bez radiusa i odbija pilke w poziomie	
			{
				Velocity.X *= -1;
				// zabiera czesc energi przez odbicie
				Velocity.X *= 0.7;
				Velocity.X += addVel;
				//Velocity.Y *= 0.8; // podczas spadania moze to wygladac dziwnie bo spowolni spadanie pilki
				Position.X = rect.Position.X - rect.Size.X / 2 - Radius - 1;
			}
			else if (Position.X > rect.Position.X + rect.Size.X / 2) // odbija w poziomie
			{
				Velocity.X *= -1;
				// zabiera czesc energi przez odbicie
				Velocity.X *= 0.7;
				Velocity.X += addVel;
				Position.X = rect.Position.X + rect.Size.X / 2 + Radius + 1;
			}
			else if (Position.Y > rect.Position.Y + rect.Size.Y / 2)
			{

				Velocity.Y *= -1;
				// zabiera czesc energi przez odbicie
				Velocity.X *= 0.8;
				Velocity.X += addVel;
				Position.Y = rect.Position.Y + rect.Size.Y / 2 + Radius + 1;
			}
			else if (Position.Y < rect.Position.Y - rect.Size.Y / 2)
			{
				Velocity.Y *= -1;
				// zabiera czesc energi przez odbicie
				Velocity.X *= 0.8;
				Velocity.X += addVel;
				Position.Y = rect.Position.Y - rect.Size.Y / 2 - Radius - 1;
			}
			return true;
		}
		return false;
	}

	bool BALL::CheckGoal(GATE gate, ALLEGRO_SAMPLE* goalSound)
	{
		if (roundActive && Position.Y >= gate.Upper.Y + Radius && Position.Y <= gate.Lower.Y - Radius)
		{
			if (Position.X <= gate.Upper.X && gate.IsLeft == true)
			{
				rightPlayerScore += 1;
				prevGoalForLeft = false;
				roundActive = false;
				al_stop_samples(); //stop dzwienkow
				al_play_sample(goalSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
				return true;
			}
			else if (Position.X >= gate.Upper.X && gate.IsLeft == false)
			{
				leftPlayerScore += 1;
				prevGoalForLeft = true;
				roundActive = false;
				al_stop_samples(); //stop dzwienkow
				al_play_sample(goalSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
				return true;
			}
		}
		return false;
	}

	bool BALL::BounceOffPlayer(float X, float Y, float headRadius, float plrSpeedX, float plrSpeedY, ALLEGRO_SAMPLE* touchSound)
	{
		// bedzie odbijac od ich glow
		VECTOR2 vec(Position.X - X, Position.Y - Y);

		float speed = Velocity.Magnitude();

		if (vec.Magnitude() <= headRadius)
		{
			al_stop_samples(); // stop dzwienkow
			al_play_sample(touchSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
			vec = vec.Unit();

			Velocity.X = vec.X * (speed + plrSpeedX)*0.85;
			Velocity.Y = vec.Y * (speed + plrSpeedY)*0.85;
			return true;
		}
		return false;
	}

	bool BALL::Kick(float x, float y, ALLEGRO_SAMPLE* touchSound)
	{
		VECTOR2 Vec(Position.X - x, Position.Y - y);

		if (Vec.Magnitude() <= 160)
		{
			Vec = Vec.Unit();

			Velocity.X = Vec.X * 30;
			Velocity.Y = Vec.Y * 30;
			al_stop_samples(); // stop dzwienkow
			al_play_sample(touchSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr); //dzwiek
			return true;
		}
		return false;
	}

}

int main()
{
	//Iniczjalizacja
	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	al_install_keyboard();
	al_install_mouse();
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(1);
	al_init_font_addon();
	al_init_ttf_addon();

	// tworzenie okien
	ALLEGRO_DISPLAY* display = al_create_display(1200, 720);
	al_set_window_title(display, "Football Heads");

	std::list<ALLEGRO_BITMAP**> bitmaps = { };

	// ładowanie obrazów dla zawodników i piłki
	ALLEGRO_BITMAP* player1_Head = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/player1_Head.png")));
	ALLEGRO_BITMAP* player1_Shoe = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/player1_Shoe.png")));
	ALLEGRO_BITMAP* player2_Head = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/player2_Head.png")));
	ALLEGRO_BITMAP* player2_Shoe = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/player2_Shoe.png")));
	ALLEGRO_BITMAP* ballBitmap = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/ball.png")));
	ALLEGRO_BITMAP* leftGateBitmap = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/left_Gate.png")));
	ALLEGRO_BITMAP* rightGateBitmap = *bitmaps.emplace_back(new ALLEGRO_BITMAP * (al_load_bitmap("img/right_Gate.png")));
	//kursor
	ALLEGRO_BITMAP* cursorBitmap = al_load_bitmap("kursor/kursorb.png");
	ALLEGRO_MOUSE_CURSOR* cursor = al_create_mouse_cursor(cursorBitmap, 0, 0);
	al_set_mouse_cursor(display, cursor);
	//animacje
	const short int maxFrame = 4;
	int curFrame = 0;
	int frameCount = 0;
	int frameDelay = 5;
	ALLEGRO_BITMAP* anim1[maxFrame];
	ALLEGRO_BITMAP* anim2[maxFrame];
	anim1[0] = al_load_bitmap("anim/Shoe1.png");
	anim1[1] = al_load_bitmap("anim/Shoe2.png");
	anim1[2] = al_load_bitmap("anim/Shoe3.png");
	anim1[3] = al_load_bitmap("anim/Shoe4.png");
	anim2[0] = al_load_bitmap("anim/Shoe1.png");
	anim2[1] = al_load_bitmap("anim/Shoe2.png");
	anim2[2] = al_load_bitmap("anim/Shoe3.png");
	anim2[3] = al_load_bitmap("anim/Shoe4.png");

	//teren
	ALLEGRO_BITMAP* teren = al_load_bitmap("img/teren.jpg");
	int groundWidth = 1200, groundHeight = 120;

	// Tworzenie czcionki do wyświetlania tekstu
	ALLEGRO_FONT* fontScore = al_load_font("font/Roboto-Medium.ttf", 50, 0);
	ALLEGRO_FONT* fontNormal = al_load_font("font/Roboto-Medium.ttf", 25, 0);

	// Tworzenie dzwięków
	ALLEGRO_SAMPLE* touchSound = al_load_sample("dzwieki/touch.wav");
	ALLEGRO_SAMPLE* goalSound = al_load_sample("dzwieki/gol.mp3");

	// sprawdzenie ładowania obrazów
	if (!player1_Head || !player2_Head || !ballBitmap || !player2_Shoe || !player1_Shoe || !anim1[0] || !cursorBitmap || !teren)
	{
		std::cout << "Nie udalo sie zaladowac obrazow\n";
		return -1;
	}
	else if (!touchSound || !goalSound)
	{
		std::cout << "Nie udalo sie zaladowac dzwiekow\n";
		return -1;
	}
	else if (!fontScore)
	{
		std::cout << "Nie udalo sie zaladowac czcionki\n";
		return -1;
	}

	// tworzenie obiektów dla zawodników i piłki
	PLAYER player1(300, 425, 0, 0);
	PLAYER player2(550, 425, 0, 0);

	player1.Y = al_get_display_height(display) - player1.Height / 2 - groundHeight;
	player1.DefaultY = player1.Y;

	player2.Y = al_get_display_height(display) - player2.Height / 2 - groundHeight;
	player2.DefaultY = player2.Y;

	phys::VECTOR2 ballPosition(500, 300);
	phys::VECTOR2 ballSpeed(0, 0);
	phys::BALL ball(ballPosition, ballSpeed, "", 50.0, -1.0);

	// nadanie predkosci poczatkowej pilki
	ball.Velocity.X = 100;
	ball.Velocity.Y = 10;
	ball.DefaultAcceleration = 1.0f;
	ball.Acceleration = 1.0;
	ball.Radius = 40.0;

	phys::VECTOR2 leftGateUpper(80.0, al_get_display_height(display) - groundHeight - 200.0);
	phys::VECTOR2 leftGateLower(80.0, al_get_display_height(display));
	phys::GATE leftGate(200.0, 80.0, true);
	leftGate.Lower = leftGateLower;
	leftGate.Upper = leftGateUpper;

	phys::VECTOR2 rightGateUpper(al_get_display_width(display) - 80.0, al_get_display_height(display) - groundHeight - 200.0);
	phys::VECTOR2 rightGateLower(al_get_display_width(display) - 80.0, al_get_display_height(display));
	phys::GATE rightGate(200.0, 80.0, false);
	rightGate.Lower = rightGateLower;
	rightGate.Upper = rightGateUpper;


	// dodatkowe obiekty na mapie - prostokaty:
	phys::VECTOR2 pozycjaPoprzeczkiLewej(leftGate.Upper.X-leftGate.Width/2, leftGate.Upper.Y);
	phys::VECTOR2 sizePoprzeczkiLewej(leftGate.Width, 10);//+10
	phys::RECTANGLE leftPoprzeczka(pozycjaPoprzeczkiLewej, sizePoprzeczkiLewej);

	al_draw_line(leftPoprzeczka.Position.X-leftPoprzeczka.Size.X/2,leftPoprzeczka.Position.Y+leftPoprzeczka.Size.Y/2 ,leftPoprzeczka.Position.X+leftPoprzeczka.Size.X/2 , leftPoprzeczka.Position.Y+leftPoprzeczka.Size.Y/2, al_map_rgb(255, 0, 0), 1);

	al_draw_line(leftPoprzeczka.Position.X - leftPoprzeczka.Size.X/2, leftPoprzeczka.Position.Y - leftPoprzeczka.Size.Y / 2, leftPoprzeczka.Position.X + leftPoprzeczka.Size.X / 2, leftPoprzeczka.Position.Y - leftPoprzeczka.Size.Y/2, al_map_rgb(255, 0, 0), 1);

	phys::VECTOR2 pozycjaPoprzeczkiPrawej(rightGate.Upper.X+rightGate.Width/2, rightGate.Upper.Y);
	phys::VECTOR2 sizePoprzeczkiPrawej(rightGate.Width, 10);//+10
	phys::RECTANGLE prawaPoprzeczka(pozycjaPoprzeczkiPrawej, sizePoprzeczkiPrawej);

	StartRound(&player1, &player2, &ball);

	// tworzenie kolejki zdarzeń i ustawianie timera
	ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0); //timer zdarzen
	ALLEGRO_TIMER* game_timer = al_create_timer(1.0); //timer gry
	int secondLeft = 60; //czas round
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_timer_event_source(game_timer));
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_start_timer(timer);
	al_start_timer(game_timer);

	bool gameOver = false;
	if (!menu(fontNormal, event_queue))
		gameOver = true;
	// cykl gry
	while (!gameOver)
	{
		// obsługa zdarzeń
		ALLEGRO_EVENT event;
		while (al_get_next_event(event_queue, &event))
		{
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			{
				gameOver = true;
			}
			else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				// przetwarzanie klawiatury dla pierwszego gracza
				if (event.keyboard.keycode == ALLEGRO_KEY_W)
				{
					player1.Jump(15.0);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_A)
				{
					player1.SpeedX = -3;
					player1.MoveLeft = true;
					player1.DrawLeft = true;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_D)
				{
					player1.SpeedX = 3;
					player1.MoveLeft = false;
					player1.DrawLeft = false;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_SPACE)
				{

					if (player1.DrawLeft)
					{
						if (ball.Position.X < player1.X)
						{
							ball.Kick(player1.X, player1.Y + player1.Height / 2, touchSound);
							
						}
					}
					else
					{
						if (ball.Position.X > player1.X)
						{
							ball.Kick(player1.X, player1.Y + player1.Height / 2, touchSound);
						}
					}

					player1.anim_started = true;
				}

				// przetwarzanie klawiatury dla drugiego gracza
				if (event.keyboard.keycode == ALLEGRO_KEY_UP)
				{
					player2.Jump(15.0);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
				{
					player2.SpeedX = -3;
					player2.MoveLeft = true;
					player2.DrawLeft = false;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
				{
					player2.SpeedX = 3;
					player2.MoveLeft = false;
					player2.DrawLeft = true;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
				{
					if (player2.DrawLeft)
					{
						
						if (ball.Position.X > player2.X)
						{
							ball.Kick(player2.X, player2.Y + player2.Height / 2, touchSound);
						}
					}
					else
					{
						if (ball.Position.X < player2.X)
						{
							ball.Kick(player2.X, player2.Y + player2.Height / 2, touchSound);

						}
					}

					player2.anim_started = true;
				}
			}
			// Przetwarzanie zwolnienia klawisza klawiatury
			else if (event.type == ALLEGRO_EVENT_KEY_UP)
			{
				// zatrzymanie gracza 2 na osi x
				if (event.keyboard.keycode == ALLEGRO_KEY_A)
				{
					player1.SpeedX = 0;
					if (!player1.MoveLeft)
					{
						player1.SpeedX = 3;
					}
				}
				if (event.keyboard.keycode == ALLEGRO_KEY_D)
				{
					player1.SpeedX = 0;
					if (player1.MoveLeft)
					{
						player1.SpeedX = -3;
					}
				}
				if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
				{
					player2.SpeedX = 0;
					if (!player2.MoveLeft)
					{
						player2.SpeedX = 3;
					}
				}
				if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
				{
					player2.SpeedX = 0;
					if (player2.MoveLeft)
					{
						player2.SpeedX = -3;
					}
				}
			}
			else if (event.type == ALLEGRO_EVENT_TIMER)// aktualizacja pozycji zawodników i piłki
			{
				if (event.timer.source == timer)
				{
					if(player1.anim_started){
						if (++frameCount >= frameDelay) {
							if (++curFrame >= maxFrame) {
								curFrame = 0;
								player1.anim_started = false;
							}
							frameCount = 0;
						}
					}
					if (player2.anim_started) {
						if (++frameCount >= frameDelay) {
							if (++curFrame >= maxFrame) {
								curFrame = 0;
								player2.anim_started = false;
							}
							frameCount = 0;
						}
					}
					player1.UpdatePos(display);
					player2.UpdatePos(display);

					//odbijanie
					ball.BounceOffPlayer(player1.X, player1.Y + player1.HeadOffset, player1.HeadRadius * 2, fabs(player1.SpeedX), fabs(player1.SpeedY), touchSound);
					ball.BounceOffPlayer(player2.X, player2.Y + player2.HeadOffset, player2.HeadRadius * 2, fabs(player2.SpeedX), fabs(player2.SpeedY), touchSound);

					ball.CollisionWithRect(leftPoprzeczka);
					ball.CollisionWithRect(prawaPoprzeczka);

					ball.Move(al_get_display_width(display), al_get_display_height(display) - groundHeight, touchSound);

					ball.CheckGoal(leftGate, goalSound);
					ball.CheckGoal(rightGate, goalSound);

					// jezeli byla gate to zaczyna runde
					StartRound(&player1, &player2, &ball);
				}
				else
					secondLeft--;
				if (secondLeft <= 0)
				{
					if (EndRound(fontNormal, event_queue))
						gameOver = true;
					else {
						secondLeft = 60;
						roundActive = false;
						rightPlayerScore = leftPlayerScore = 0;
						StartRound(&player1, &player2, &ball);
					}
				}
			}
		}
		// rysowanie obiektów na ekranie
		al_clear_to_color(al_map_rgb(255, 255, 255));
		//WYNIKI GRY
		al_draw_textf(fontScore, al_map_rgb(0, 0, 0), 535, 100, ALLEGRO_ALIGN_CENTRE, "%d", leftPlayerScore);
		al_draw_text(fontScore, al_map_rgb(0, 0, 0), 575, 100, ALLEGRO_ALIGN_CENTRE, ":");
		al_draw_textf(fontScore, al_map_rgb(0, 0, 0), 615, 100, ALLEGRO_ALIGN_CENTRE, "%d", rightPlayerScore);
		al_draw_text(fontNormal, al_map_rgb(0, 0, 0), 10, 10, 0, "Time: ");
		al_draw_textf(fontNormal, al_map_rgb(0, 0, 0), 100, 10, 0, "%d", secondLeft);
		//Textury
		//al_draw_filled_rectangle(groundX, groundY, groundX + groundWidth, groundY + groundHeight, groundColor); //teren
		
		//al_draw_line(leftPoprzeczka.Position.X - leftPoprzeczka.Size.X / 2, leftPoprzeczka.Position.Y + leftPoprzeczka.Size.Y / 2, leftPoprzeczka.Position.X + leftPoprzeczka.Size.X / 2, leftPoprzeczka.Position.Y + leftPoprzeczka.Size.Y / 2, al_map_rgb(255, 0, 0), 1);
		//al_draw_line(leftPoprzeczka.Position.X - leftPoprzeczka.Size.X / 2, leftPoprzeczka.Position.Y - leftPoprzeczka.Size.Y / 2, leftPoprzeczka.Position.X + leftPoprzeczka.Size.X / 2, leftPoprzeczka.Position.Y - leftPoprzeczka.Size.Y / 2, al_map_rgb(255, 0, 0), 1);
		
		al_draw_scaled_bitmap(teren, 0, 0, al_get_bitmap_width(teren), al_get_bitmap_height(teren), 0, al_get_display_height(display) - groundHeight, groundWidth, groundHeight, 0);//teren
		al_draw_scaled_bitmap(leftGateBitmap, 0, 0, al_get_bitmap_width(leftGateBitmap), al_get_bitmap_height(leftGateBitmap), 0, al_get_display_height(display) - leftGate.Height - groundHeight, leftGate.Width, leftGate.Height, 0);
		al_draw_scaled_bitmap(rightGateBitmap, 0, 0, al_get_bitmap_width(rightGateBitmap), al_get_bitmap_height(rightGateBitmap), al_get_display_width(display) - rightGate.Width, al_get_display_height(display) - rightGate.Height - groundHeight, leftGate.Width, leftGate.Height, 0);
		if (player1.DrawLeft) {
			al_draw_scaled_bitmap(player1_Head, 0, 0, al_get_bitmap_width(player1_Head), al_get_bitmap_height(player1_Head), player1.X - player1.HeadRadius, player1.Y + player1.HeadOffset - player1.HeadRadius, player1.HeadRadius * 2, player1.HeadRadius * 2, 0);
			if (player1.anim_started)
				al_draw_scaled_bitmap(anim1[curFrame], 0, 0, al_get_bitmap_width(player1_Shoe), al_get_bitmap_height(player1_Shoe), player1.X - player1.ShoeWidth / 2, player1.Y + player1.Height / 2 - player1.ShoeHeight / 2 + player1.ShoeOffset, player1.ShoeWidth, player1.ShoeHeight, 0);
			else
				al_draw_scaled_bitmap(player1_Shoe, 0, 0, al_get_bitmap_width(player1_Shoe), al_get_bitmap_height(player1_Shoe), player1.X - player1.ShoeWidth / 2, player1.Y + player1.Height / 2 - player1.ShoeHeight / 2 + player1.ShoeOffset, player1.ShoeWidth, player1.ShoeHeight, 0);
		}
		else {
			al_draw_scaled_bitmap(player1_Head, 0, 0, al_get_bitmap_width(player1_Head), al_get_bitmap_height(player1_Head), player1.X - player1.HeadRadius, player1.Y + player1.HeadOffset - player1.HeadRadius, player1.HeadRadius * 2, player1.HeadRadius * 2, ALLEGRO_FLIP_HORIZONTAL);
			if (player1.anim_started)
				al_draw_scaled_bitmap(anim1[curFrame], 0, 0, al_get_bitmap_width(player1_Shoe), al_get_bitmap_height(player1_Shoe), player1.X - player1.ShoeWidth / 2, player1.Y + player1.Height / 2 - player1.ShoeHeight / 2 + player1.ShoeOffset, player1.ShoeWidth, player1.ShoeHeight, ALLEGRO_FLIP_HORIZONTAL);
			else
				al_draw_scaled_bitmap(player1_Shoe, 0, 0, al_get_bitmap_width(player1_Shoe), al_get_bitmap_height(player1_Shoe), player1.X - player1.ShoeWidth / 2, player1.Y + player1.Height / 2 - player1.ShoeHeight / 2 + player1.ShoeOffset, player1.ShoeWidth, player1.ShoeHeight, ALLEGRO_FLIP_HORIZONTAL);
		}
		if (player2.DrawLeft) {
			al_draw_scaled_bitmap(player2_Head, 0, 0, al_get_bitmap_width(player2_Head), al_get_bitmap_height(player2_Head), player2.X - player2.HeadRadius, player2.Y + player2.HeadOffset - player2.HeadRadius, player2.HeadRadius * 2, player2.HeadRadius * 2, ALLEGRO_FLIP_HORIZONTAL);
			if (player2.anim_started)
				al_draw_scaled_bitmap(anim2[curFrame], 0, 0, al_get_bitmap_width(player2_Shoe), al_get_bitmap_height(player2_Shoe), player2.X - player2.ShoeWidth / 2, player2.Y + player2.Height / 2 - player2.ShoeHeight / 2 + player2.ShoeOffset, player2.ShoeWidth, player2.ShoeHeight, ALLEGRO_FLIP_HORIZONTAL);
			else
				al_draw_scaled_bitmap(player2_Shoe, 0, 0, al_get_bitmap_width(player2_Shoe), al_get_bitmap_height(player2_Shoe), player2.X - player2.ShoeWidth / 2, player2.Y + player2.Height / 2 - player2.ShoeHeight / 2 + player2.ShoeOffset, player2.ShoeWidth, player2.ShoeHeight, ALLEGRO_FLIP_HORIZONTAL);
		}
		else {
			al_draw_scaled_bitmap(player2_Head, 0, 0, al_get_bitmap_width(player2_Head), al_get_bitmap_height(player2_Head), player2.X - player2.HeadRadius, player2.Y + player2.HeadOffset - player2.HeadRadius, player2.HeadRadius * 2, player2.HeadRadius * 2, 0);
			if (player2.anim_started)
				al_draw_scaled_bitmap(anim2[curFrame], 0, 0, al_get_bitmap_width(player2_Shoe), al_get_bitmap_height(player2_Shoe), player2.X - player2.ShoeWidth / 2, player2.Y + player2.Height / 2 - player2.ShoeHeight / 2 + player2.ShoeOffset, player2.ShoeWidth, player2.ShoeHeight, 0);
			else
				al_draw_scaled_bitmap(player2_Shoe, 0, 0, al_get_bitmap_width(player2_Shoe), al_get_bitmap_height(player2_Shoe), player2.X - player2.ShoeWidth / 2, player2.Y + player2.Height / 2 - player2.ShoeHeight / 2 + player2.ShoeOffset, player2.ShoeWidth, player2.ShoeHeight, 0);

		}
		al_draw_scaled_bitmap(ballBitmap, 0, 0, al_get_bitmap_width(ballBitmap), al_get_bitmap_height(ballBitmap), ball.Position.X - ball.Radius, ball.Position.Y - ball.Radius, ball.Radius * 2, ball.Radius * 2, 0); //al_get_bitmap_height(ballBitmap) / 50

		al_flip_display();
	}

	// uwalnianie zasobów
	for (int i = 0; i < maxFrame; i++) {
		al_destroy_bitmap(anim1[i]);
		al_destroy_bitmap(anim2[i]);
	}
	al_destroy_bitmap(cursorBitmap);
	al_destroy_mouse_cursor(cursor);
	al_destroy_font(fontScore);
	al_destroy_font(fontNormal);
	al_destroy_sample(goalSound);
	al_destroy_sample(touchSound);
	al_destroy_bitmap(player1_Head);
	al_destroy_bitmap(player1_Shoe);
	al_destroy_bitmap(player2_Head);
	al_destroy_bitmap(player2_Shoe);
	al_destroy_bitmap(ballBitmap);
	al_destroy_bitmap(leftGateBitmap);
	al_destroy_bitmap(rightGateBitmap);
	al_destroy_timer(timer);
	al_destroy_timer(game_timer);
	al_destroy_event_queue(event_queue);
	al_destroy_display(display);
	al_destroy_bitmap(teren);
	return 0;
}