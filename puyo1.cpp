//課題

#include <curses.h>
#include <stdlib.h> // rand関数

// 左側のぷよが最初に生成される位置．
// もう一つは(x+1, y)の位置に生成される．
#define RESPAWN_X 5
#define RESPAWN_Y 0

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

//「落下中」ぷよを管理するクラス．PuyoArrayクラスをpublicで継承
class PuyoArrayActive : public PuyoArray
{
private:
	int puyorotate;

public:
	// コンストラクタ
	PuyoArrayActive()
	{
		this->puyorotate = 0;
	}

	void set_rotate(int new_rotate)
	{
		this->puyorotate = new_rotate;
	}

	int get_rotate()
	{
		return this->puyorotate;
	}
};

//「着地済み」ぷよを管理するクラス．PuyoArrayクラスをpublicで継承
class PuyoArrayStack : public PuyoArray
{
};

class PuyoControl
{
private:
	bool landed; // 一度着地したら次のぷよが生成される(GeneratedPuyo呼び出し)まで着地判定を保持する．

public:
	// ランダムにぷよを選択(GeneratePuyoから呼ばれる)
	puyocolor RandomSelectPuyo() const
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
	void GeneratePuyo(PuyoArrayActive &active)
	{
		this->landed = false;

		puyocolor newpuyo1;
		newpuyo1 = this->RandomSelectPuyo();

		puyocolor newpuyo2;
		newpuyo2 = this->RandomSelectPuyo();

		active.SetValue(RESPAWN_Y, RESPAWN_X, newpuyo1);
		active.SetValue(RESPAWN_Y, RESPAWN_X + 1, newpuyo2);
		active.set_rotate(0); // puyoの回転状態の初期値
	}

	//ぷよの着地判定．着地判定があるとtrueを返す
	// 着地時にぷよを消すので，constにできない．
	bool LandingPuyo(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		// if文をわかりやすくするために変数で管理
		bool exist_puyo = false;
		bool landing_on_gnd = false;
		bool landing_on_puyo = false;

		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				exist_puyo = active.GetValue(y, x) != NONE;
				landing_on_gnd = y == active.GetLine() - 1;
				landing_on_puyo = stack.GetValue(y + 1, x) != NONE;
				if (exist_puyo && (landing_on_gnd || landing_on_puyo))
				{
					this->landed = true;

					// activeのぷよのどちらかが着地すれば，activeの2つのぷよと同じ位置にstackを作る
					// ぷよは必ず隣り合っているので探索範囲は狭い
					for (int j = y - 1; j < y + 2; j++)
					{
						for (int i = x - 1; i < x + 2; i++)
						{
							// すでにあるstackを上書きしないようにする
							if (active.GetValue(j, i) != NONE)
							{
								stack.SetValue(j, i, active.GetValue(j, i));
								active.SetValue(j, i, NONE);
							}
						}
					}
				}
			}
		}

		// リスポーン地点にstackぷよがあるときは新たに生成させない．
		if (stack.GetValue(RESPAWN_Y, RESPAWN_X) != NONE || stack.GetValue(RESPAWN_Y, RESPAWN_X + 1) != NONE)
		{
			this->landed = false;
		}

		return this->landed;
	}

	//左移動
	void MoveLeft(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[active.GetSize()];

		for (int i = 0; i < active.GetSize(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ左の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				if (active.GetValue(y, x) == NONE)
				{
					continue;
				}

				// 先に左側にあるぷよが判定される．
				// 左側のぷよを先に左へ動かし，あった場所をNONEにする
				// 右側にぷよがあったとき，そのぷよの左側はすでにNONEになっているので，判定にヒットする．
				// stackぷよがあるときには処理は行わない．
				if (0 < x && active.GetValue(y, x - 1) == NONE && stack.GetValue(y, x - 1) == NONE)
				{
					puyo_temp[y * active.GetColumn() + (x - 1)] = active.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					active.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * active.GetColumn() + x] = active.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				active.SetValue(y, x, puyo_temp[y * active.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//右移動
	void MoveRight(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[active.GetSize()];

		for (int i = 0; i < active.GetSize(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ右の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = active.GetColumn() - 1; x >= 0; x--)
			{
				if (active.GetValue(y, x) == NONE)
				{
					continue;
				}

				if (x < active.GetColumn() - 1 && active.GetValue(y, x + 1) == NONE && stack.GetValue(y, x + 1) == NONE)
				{
					puyo_temp[y * active.GetColumn() + (x + 1)] = active.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					active.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * active.GetColumn() + x] = active.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				active.SetValue(y, x, puyo_temp[y * active.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//下移動
	void MoveDown(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[active.GetSize()];

		for (int i = 0; i < active.GetSize(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ下の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = active.GetLine() - 1; y >= 0; y--)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				if (active.GetValue(y, x) == NONE)
				{
					continue;
				}

				if (y < active.GetLine() - 1 && active.GetValue(y + 1, x) == NONE && stack.GetValue(y + 1, x) == NONE)
				{
					puyo_temp[(y + 1) * active.GetColumn() + x] = active.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					active.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * active.GetColumn() + x] = active.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				active.SetValue(y, x, puyo_temp[y * active.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	/*
	NUCTのpuyo4_vanish.cppを引用
	*/
	//ぷよ消滅処理を全座標で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &puyostack)
	{
		int vanishednumber = 0;
		for (int y = 0; y < puyostack.GetLine(); y++)
		{
			for (int x = 0; x < puyostack.GetColumn(); x++)
			{
				vanishednumber += VanishPuyo(puyostack, y, x);
			}
		}

		return vanishednumber;
	}

	//ぷよ消滅処理を座標(x,y)で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &puyostack, unsigned int y, unsigned int x)
	{
		//判定個所にぷよがなければ処理終了
		if (puyostack.GetValue(y, x) == NONE)
		{
			return 0;
		}

		//判定状態を表す列挙型
		//NOCHECK判定未実施，CHECKINGが判定対象，CHECKEDが判定済み
		enum checkstate
		{
			NOCHECK,
			CHECKING,
			CHECKED
		};

		//判定結果格納用の配列
		enum checkstate *field_array_check;
		field_array_check = new enum checkstate[puyostack.GetLine() * puyostack.GetColumn()];

		//配列初期化
		for (int i = 0; i < puyostack.GetLine() * puyostack.GetColumn(); i++)
		{
			field_array_check[i] = NOCHECK;
		}

		//座標(x,y)を判定対象にする
		field_array_check[y * puyostack.GetColumn() + x] = CHECKING;

		//判定対象が1つもなくなるまで，判定対象の上下左右に同じ色のぷよがあるか確認し，あれば新たな判定対象にする
		bool checkagain = true;
		while (checkagain)
		{
			checkagain = false;

			for (int y = 0; y < puyostack.GetLine(); y++)
			{
				for (int x = 0; x < puyostack.GetColumn(); x++)
				{
					//(x,y)に判定対象がある場合
					if (field_array_check[y * puyostack.GetColumn() + x] == CHECKING)
					{
						//(x+1,y)の判定
						if (x < puyostack.GetColumn() - 1)
						{
							//(x+1,y)と(x,y)のぷよの色が同じで，(x+1,y)のぷよが判定未実施か確認
							if (puyostack.GetValue(y, x + 1) == puyostack.GetValue(y, x) && field_array_check[y * puyostack.GetColumn() + (x + 1)] == NOCHECK)
							{
								//(x+1,y)を判定対象にする
								field_array_check[y * puyostack.GetColumn() + (x + 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x-1,y)の判定
						if (x > 0)
						{
							if (puyostack.GetValue(y, x - 1) == puyostack.GetValue(y, x) && field_array_check[y * puyostack.GetColumn() + (x - 1)] == NOCHECK)
							{
								field_array_check[y * puyostack.GetColumn() + (x - 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y+1)の判定
						if (y < puyostack.GetLine() - 1)
						{
							if (puyostack.GetValue(y + 1, x) == puyostack.GetValue(y, x) && field_array_check[(y + 1) * puyostack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y + 1) * puyostack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y-1)の判定
						if (y > 0)
						{
							if (puyostack.GetValue(y - 1, x) == puyostack.GetValue(y, x) && field_array_check[(y - 1) * puyostack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y - 1) * puyostack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y)を判定済みにする
						field_array_check[y * puyostack.GetColumn() + x] = CHECKED;
					}
				}
			}
		}

		//判定済みの数をカウント
		int puyocount = 0;
		for (int i = 0; i < puyostack.GetLine() * puyostack.GetColumn(); i++)
		{
			if (field_array_check[i] == CHECKED)
			{
				puyocount++;
			}
		}

		//4個以上あれば，判定済み座標のぷよを消す
		int vanishednumber = 0;
		if (4 <= puyocount)
		{
			for (int y = 0; y < puyostack.GetLine(); y++)
			{
				for (int x = 0; x < puyostack.GetColumn(); x++)
				{
					if (field_array_check[y * puyostack.GetColumn() + x] == CHECKED)
					{
						puyostack.SetValue(y, x, NONE);

						vanishednumber++;
					}
				}
			}
		}

		//メモリ解放
		delete[] field_array_check;

		return vanishednumber;
	}

	//回転
	//PuyoArrayActiveクラスのprivateメンバ変数として int puyorotate を宣言し，これに回転状態を記憶させている．
	//puyorotateにはコンストラクタ及びGeneratePuyo関数で値0を代入する必要あり．
	void Rotate(PuyoArrayActive &puyoactive, PuyoArrayStack &stack)
	{
		//フィールドをラスタ順に探索（最も上の行を左から右方向へチェックして，次に一つ下の行を左から右方向へチェックして，次にその下の行・・と繰り返す）し，先に発見される方をpuyo1, 次に発見される方をpuyo2に格納
		puyocolor puyo1, puyo2;
		int puyo1_x = 0;
		int puyo1_y = 0;
		int puyo2_x = 0;
		int puyo2_y = 0;

		bool findingpuyo1 = true;
		for (int y = 0; y < puyoactive.GetLine(); y++)
		{
			for (int x = 0; x < puyoactive.GetColumn(); x++)
			{
				if (puyoactive.GetValue(y, x) != NONE)
				{
					if (findingpuyo1)
					{
						puyo1 = puyoactive.GetValue(y, x);
						puyo1_x = x;
						puyo1_y = y;
						findingpuyo1 = false;
					}
					else
					{
						puyo2 = puyoactive.GetValue(y, x);
						puyo2_x = x;
						puyo2_y = y;
					}
				}
			}
		}

		//回転前のぷよを消す
		puyoactive.SetValue(puyo1_y, puyo1_x, NONE);
		puyoactive.SetValue(puyo2_y, puyo2_x, NONE);

		//操作中ぷよの回転
		switch (puyoactive.get_rotate())
		{
		case 0:
			//回転パターン
			//RB -> R
			//      B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y >= puyoactive.GetLine() - 1) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
			puyoactive.SetValue(puyo2_y + 1, puyo2_x - 1, puyo2);
			//次の回転パターンの設定
			puyoactive.set_rotate(1);
			break;

		case 1:
			//回転パターン
			//R -> BR
			//B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y <= 0) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
			puyoactive.SetValue(puyo2_y - 1, puyo2_x - 1, puyo2);

			//次の回転パターンの設定
			puyoactive.set_rotate(2);
			break;

		case 2:
			//回転パターン
			//      B
			//BR -> R
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x >= puyoactive.GetColumn() - 1 || puyo1_y <= 0) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y - 1, puyo1_x + 1, puyo1);
			puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyoactive.set_rotate(3);
			break;

		case 3:
			//回転パターン
			//B
			//R -> RB
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x >= puyoactive.GetColumn() - 1 || puyo1_y >= puyoactive.GetLine() - 1) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y + 1, puyo1_x + 1, puyo1);
			puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyoactive.set_rotate(0);
			break;

		default:
			break;
		}

		// 回転後にすでにあるぷよ(stack)と衝突するなら，回転を取り消す．
		for (int y = 0; y < stack.GetLine(); y++)
		{
			for (int x = 0; x < stack.GetColumn(); x++)
			{
				if (stack.GetValue(y, x) != NONE)
				{
					// (x,y)にstackぷよが存在している
					if (puyoactive.GetValue(y, x) != NONE)
					{
						// (x,y)にactiveぷよが存在している(衝突している)
						puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
						puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
					}
				}
			}
		}
	}

	// 宙に浮いたぷよを落下させる
	bool DropFloatingPuyo(PuyoArrayStack &stack)
	{
		bool lock = true;
		for (int x = 0; x < stack.GetColumn(); x++)
		{
			for (int y = 0; y < stack.GetLine() - 1; y++)
			{
				if ((stack.GetValue(y, x) != NONE) && stack.GetValue(y + 1, x) == NONE)
				{
					// 存在する(NONEでない)ぷよの下が空いている(NONE)
					lock = true;
					stack.SetValue(y + 1, x, stack.GetValue(y, x));
					stack.SetValue(y, x, NONE);
				}
				else
				{
					lock = false;
				}
			}
		}
		return lock;
	}

	bool GameOver(PuyoArrayStack &stack)
	{
		bool flag = false;

		for (int x = 0; x < stack.GetColumn(); x++)
		{
			if (stack.GetValue(0, x) != NONE)
			{
				// 盤面頂上にぷよがあるときはゲームオーバー
				flag = true;
			}
		}

		return flag;
	}
};

void DisplayPuyo(PuyoArray &puyo, int y, int x)
{
	// PuyoArrayActiveもPuyoArrayStackもPuyoArrayにキャストして表示する
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
void Display(PuyoArrayActive &active, PuyoArrayStack &stack, PuyoControl &control, int score, int rensa)
{
	//落下中ぷよ表示
	for (int y = 0; y < active.GetLine(); y++)
	{
		for (int x = 0; x < active.GetColumn(); x++)
		{
			// 互いに上書きし合う．
			// activeが存在する(!=NONE)ときはそちらを表示し，全体的な表示はstackを優先する．
			if (active.GetValue(y, x) != NONE)
			{
				DisplayPuyo(active, y, x);
			}
			else
			{
				DisplayPuyo(stack, y, x);
			}
		}
	}

	//情報表示
	int count = 0;
	for (int y = 0; y < active.GetLine(); y++)
	{
		for (int x = 0; x < active.GetColumn(); x++)
		{
			if (active.GetValue(y, x) != NONE || stack.GetValue(y, x) != NONE)
			{
				count++;
			}
		}
	}

	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(5, COLOR_YELLOW, COLOR_RED);

	// ゲームオーバー表示
	if (control.GameOver(stack))
	{
		char msg_gameover[256];
		attrset(COLOR_PAIR(5));
		sprintf(msg_gameover, "GAME OVER");
		mvaddstr(4, COLS - 25, msg_gameover);
	}

	attrset(COLOR_PAIR(0));

	char msg[256];
	sprintf(msg, "Field: %d x %d, Puyo number: %03d", active.GetLine(), active.GetColumn(), count);
	mvaddstr(2, COLS - 35, msg);

	//スコア表示
	char msg_score[256];
	sprintf(msg_score, "Score: %d", score);
	mvaddstr(8, COLS - 35, msg_score);

	// 連鎖表示
	char msg_rensa[256];
	sprintf(msg_rensa, "Rensa: %d", rensa);
	mvaddstr(9, COLS - 35, msg_rensa);

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

	// 盤面状態管理
	// PuyoArray puyo;
	PuyoArrayActive active;
	PuyoArrayStack stack;
	// Generate, Landing, MoveLeft, MoveRight, MoveDown
	PuyoControl control;

	int score = 0;
	int vanished = 0;
	int rensa = 0;

	// 初期化処理
	active.ChangeSize(LINES / 2, COLS / 2); //フィールドは画面サイズの縦横1/2にする
	stack.ChangeSize(LINES / 2, COLS / 2);	//フィールドは画面サイズの縦横1/2にする
	control.GeneratePuyo(active);

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
		// 要改善：ぷよを横から差し込んだときに片方だけ降下を続けてしまう．
		switch (ch)
		{
		case KEY_LEFT:
			control.MoveLeft(active, stack);
			break;
		case KEY_RIGHT:
			control.MoveRight(active, stack);
			break;
		case 'z':
			//ぷよ回転処理
			control.Rotate(active, stack);
			break;
		default:
			break;
		}

		//処理速度調整のためのif文
		if (delay % waitCount == 0)
		{
			if (!control.LandingPuyo(active, stack))
			{
				//ぷよ下に移動
				control.MoveDown(active, stack);
			}

			//ぷよ着地判定
			// 横から差し込んで着地した場合には，Downが実行されず，すでにactiveぷよがない．
			// メンバ変数landedに着地情報が保持されているので，そちらが利用される．
			if (control.LandingPuyo(active, stack))
			{
				// 4つ以上つながったぷよを削除
				// 削除したぷよの数だけ得点になる．
				vanished = control.VanishPuyo(stack);
				score += vanished;

				// 連鎖回数*10の得点を加える．
				if (vanished > 0)
				{
					rensa += 1;
				}
				else
				{
					rensa = 0;
				}
				if (rensa > 1)
				{
					score += (rensa - 1) * 10;
				}

				// ゲームオーバーではなく着地していたら新しいぷよ生成
				control.GeneratePuyo(active);
			}
		}
		if (!control.DropFloatingPuyo(stack))
		{
			delay++;
		}
		//表示
		bool is_gameover = control.GameOver(stack);
		Display(active, stack, control, score, rensa);
	}

	//画面をリセット
	endwin();

	return 0;
}
