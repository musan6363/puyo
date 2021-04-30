//課題
//2020/04/17

#include <curses.h>
#include <stdlib.h> // rand関数

//ぷよの色を表すの列挙型
//NONEが無し，RED,BLUE,..が色を表す
enum puyocolor
{
	NONE,
	RED,
	BLUE,
	GREEN,
	YELLOW
};

class PuyoArray
{
private:
	//盤面状態
	puyocolor *data;
	unsigned int data_line;
	unsigned int data_column;
	unsigned int data_size; // line*column

	//メモリ開放
	void Release()
	{
		if (this->data == NULL)
		{
			return;
		}

		delete[] this->data;
		this->data = NULL;
	}

public:
	// コンストラクタ
	PuyoArray()
	{
		this->data = NULL;
		this->data_line = 0;
		this->data_column = 0;
		this->data_size = 0;
	}

	// デストラクタ
	~PuyoArray()
	{
		this->Release();
	}

	//盤面サイズ変更
	void ChangeSize(unsigned int line, unsigned int column)
	{
		this->Release();

		this->data_line = line;
		this->data_column = column;
		this->data_size = line * column;

		//新しいサイズでメモリ確保
		this->data = new puyocolor[this->data_size];
	}

	//盤面の行数を返す
	unsigned int GetLine() const
	{
		return this->data_line;
	}

	//盤面の列数を返す
	unsigned int GetColumn() const
	{
		return this->data_column;
	}

	// 盤面のサイズ(行数x列数)を返す．追加実装．
	unsigned int GetSize() const
	{
		// メンバ変数を増やさずに，
		// return this->data_column * this->data_line;
		// としても良い．
		// その場合，コンストラクタ等での初期化も削除が必要．
		return this->data_size;
	}

	//盤面の指定された位置の値を返す
	puyocolor GetValue(unsigned int y, unsigned int x) const
	{
		if (y >= this->GetLine() || x >= this->GetColumn())
		{
			//引数の値が正しくない
			return NONE;
		}

		return this->data[y * this->GetColumn() + x];
	}

	//盤面の指定された位置に値を書き込む
	void SetValue(unsigned int y, unsigned int x, puyocolor value)
	{
		if (y >= this->GetLine() || x >= this->GetColumn())
		{
			//引数の値が正しくない
			return;
		}

		data[y * this->GetColumn() + x] = value;
	}
};

// ランダムにぷよを選択(GeneratePuyoから呼ばれる)
puyocolor RandomSelectPuyo()
{
	puyocolor newpuyo;
	// puyocolorからランダムに選択する．
	// YELLOWは末尾の要素でenumの長さを調べるために利用．
	// NONE以外のぷよを返す．
	do
	{
		newpuyo = static_cast<puyocolor>(rand() % (YELLOW + 1));
	} while (newpuyo == NONE);

	return newpuyo;
}

//盤面に新しいぷよ生成
void GeneratePuyo(PuyoArray &puyo)
{
	puyocolor newpuyo1;
	newpuyo1 = RandomSelectPuyo();

	puyocolor newpuyo2;
	newpuyo2 = RandomSelectPuyo();

	puyo.SetValue(0, 5, newpuyo1);
	puyo.SetValue(0, 6, newpuyo2);
}

//ぷよの着地判定．着地判定があるとtrueを返す
// 着地時にぷよを消すので，constにできない．
bool LandingPuyo(PuyoArray &puyo)
{
	bool landed = false;

	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			if (puyo.GetValue(y, x) != NONE && y == puyo.GetLine() - 1)
			{
				landed = true;

				//着地判定されたぷよを消す．本処理は必要に応じて変更する．
				puyo.SetValue(y, x, NONE);
			}
		}
	}

	return landed;
}

//左移動
void MoveLeft(PuyoArray &puyo)
{
	//一時的格納場所メモリ確保
	puyocolor *puyo_temp = new puyocolor[puyo.GetSize()];

	for (int i = 0; i < puyo.GetSize(); i++)
	{
		puyo_temp[i] = NONE;
	}

	//1つ左の位置にpuyoactiveからpuyo_tempへとコピー
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			if (puyo.GetValue(y, x) == NONE)
			{
				continue;
			}

			if (0 < x && puyo.GetValue(y, x - 1) == NONE)
			{
				puyo_temp[y * puyo.GetColumn() + (x - 1)] = puyo.GetValue(y, x);
				//コピー後に元位置のpuyoactiveのデータは消す
				puyo.SetValue(y, x, NONE);
			}
			else
			{
				puyo_temp[y * puyo.GetColumn() + x] = puyo.GetValue(y, x);
			}
		}
	}

	//puyo_tempからpuyoactiveへコピー
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			puyo.SetValue(y, x, puyo_temp[y * puyo.GetColumn() + x]);
		}
	}

	//一時的格納場所メモリ解放
	delete[] puyo_temp;
}

//右移動
void MoveRight(PuyoArray &puyo)
{
	//一時的格納場所メモリ確保
	puyocolor *puyo_temp = new puyocolor[puyo.GetSize()];

	for (int i = 0; i < puyo.GetSize(); i++)
	{
		puyo_temp[i] = NONE;
	}

	//1つ右の位置にpuyoactiveからpuyo_tempへとコピー
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = puyo.GetColumn() - 1; x >= 0; x--)
		{
			if (puyo.GetValue(y, x) == NONE)
			{
				continue;
			}

			if (x < puyo.GetColumn() - 1 && puyo.GetValue(y, x + 1) == NONE)
			{
				puyo_temp[y * puyo.GetColumn() + (x + 1)] = puyo.GetValue(y, x);
				//コピー後に元位置のpuyoactiveのデータは消す
				puyo.SetValue(y, x, NONE);
			}
			else
			{
				puyo_temp[y * puyo.GetColumn() + x] = puyo.GetValue(y, x);
			}
		}
	}

	//puyo_tempからpuyoactiveへコピー
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			puyo.SetValue(y, x, puyo_temp[y * puyo.GetColumn() + x]);
		}
	}

	//一時的格納場所メモリ解放
	delete[] puyo_temp;
}

//下移動
void MoveDown(PuyoArray &puyo)
{
	//一時的格納場所メモリ確保
	puyocolor *puyo_temp = new puyocolor[puyo.GetSize()];

	for (int i = 0; i < puyo.GetSize(); i++)
	{
		puyo_temp[i] = NONE;
	}

	//1つ下の位置にpuyoactiveからpuyo_tempへとコピー
	for (int y = puyo.GetLine() - 1; y >= 0; y--)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			if (puyo.GetValue(y, x) == NONE)
			{
				continue;
			}

			if (y < puyo.GetLine() - 1 && puyo.GetValue(y + 1, x) == NONE)
			{
				puyo_temp[(y + 1) * puyo.GetColumn() + x] = puyo.GetValue(y, x);
				//コピー後に元位置のpuyoactiveのデータは消す
				puyo.SetValue(y, x, NONE);
			}
			else
			{
				puyo_temp[y * puyo.GetColumn() + x] = puyo.GetValue(y, x);
			}
		}
	}

	//puyo_tempからpuyoactiveへコピー
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			puyo.SetValue(y, x, puyo_temp[y * puyo.GetColumn() + x]);
		}
	}

	//一時的格納場所メモリ解放
	delete[] puyo_temp;
}

void DisplayPuyo(PuyoArray &puyo, int y, int x)
{
	// 改善点: 毎回色の定義を行うのは非効率．
	switch (puyo.GetValue(y, x))
	{
	case NONE:
		init_pair(0, COLOR_WHITE, COLOR_BLACK);
		attrset(COLOR_PAIR(0));
		mvaddch(y, x, '.');
		break;
	case RED:
		init_pair(1, COLOR_RED, COLOR_BLACK);
		attrset(COLOR_PAIR(1));
		mvaddch(y, x, 'R');
		break;
	case BLUE:
		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		attrset(COLOR_PAIR(2));
		mvaddch(y, x, 'B');
		break;
	case GREEN:
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		attrset(COLOR_PAIR(3));
		mvaddch(y, x, 'G');
		break;
	case YELLOW:
		init_pair(4, COLOR_YELLOW, COLOR_BLACK);
		attrset(COLOR_PAIR(4));
		mvaddch(y, x, 'Y');
		break;
	default:
		mvaddch(y, x, '?');
		break;
	}
}

//表示
void Display(PuyoArray &puyo)
{
	//落下中ぷよ表示
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			DisplayPuyo(puyo, y, x);
		}
	}

	//情報表示
	int count = 0;
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
			if (puyo.GetValue(y, x) != NONE)
			{
				count++;
			}
		}
	}

	char msg[256];
	sprintf(msg, "Field: %d x %d, Puyo number: %03d", puyo.GetLine(), puyo.GetColumn(), count);
	mvaddstr(2, COLS - 35, msg);

	refresh();
}

//ここから実行される
int main(int argc, char **argv)
{
	//画面の初期化
	initscr();
	//カラー属性を扱うための初期化
	start_color();

	//キーを押しても画面に表示しない
	noecho();
	//キー入力を即座に受け付ける
	cbreak();

	curs_set(0);
	//キー入力受付方法指定
	keypad(stdscr, TRUE);

	//キー入力非ブロッキングモード
	timeout(0);

	//初期化処理
	PuyoArray puyo;
	puyo.ChangeSize(LINES / 2, COLS / 2); //フィールドは画面サイズの縦横1/2にする
	GeneratePuyo(puyo);

	int delay = 0;
	int waitCount = 20000;

	int puyostate = 0;

	//メイン処理ループ
	while (1)
	{
		//キー入力受付
		int ch;
		ch = getch();

		//Qの入力で終了
		if (ch == 'Q')
		{
			break;
		}

		//入力キーごとの処理
		switch (ch)
		{
		case KEY_LEFT:
			MoveLeft(puyo);
			break;
		case KEY_RIGHT:
			MoveRight(puyo);
			break;
		case 'z':
			//ぷよ回転処理
			break;
		default:
			break;
		}

		//処理速度調整のためのif文
		if (delay % waitCount == 0)
		{
			//ぷよ下に移動
			MoveDown(puyo);

			//ぷよ着地判定
			if (LandingPuyo(puyo))
			{
				//着地していたら新しいぷよ生成
				GeneratePuyo(puyo);
			}
		}
		delay++;

		//表示
		Display(puyo);
	}

	//画面をリセット
	endwin();

	return 0;
}
