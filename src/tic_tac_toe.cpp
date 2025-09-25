#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace std;

// Classe TicTacToe
class TicTacToe {
  private:
  std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
  char current_player; // Jogador atual ('X' ou 'O')
  bool game_over; // Estado do jogo
  char winner; // Vencedor do jogo

  std::mutex mtx; // Mutex para proteger o tabuleiro
  std::condition_variable cv; // Variável de condição para alternância de turnos
  
  public:
  TicTacToe() {
    // Inicializar o tabuleiro e as variáveis do jogo
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        board[i][j] = ' ';
      }
    }
    winner = '-';
    game_over = 0;
    // sorteia jogador inicial entre 'X' e 'O'
    static std::mt19937 sorteiaJogador(static_cast<unsigned int>(time(0)));
    static std::uniform_int_distribution<int> distr(0, 1);
    current_player = distr(sorteiaJogador) == 0 ? 'X' : 'O';
  }
  
  void display_board() {
  std::cout << "\033[2J\033[1;1H"; // Limpar tela
  std::cout << "Tabuleiro:\n\n";

  for (int i = 0; i < 3; i++) {
    std::cout << " ";
    for (int j = 0; j < 3; j++) {
      std::cout << " " << board[i][j] << " ";
      if (j < 2) std::cout << "|";
    }
    std::cout << "\n";
    if (i < 2) std::cout << "-------------\n";
  }

  std::cout << "\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
}
  
  bool make_move(char player, int row, int col) {
    std::unique_lock<std::mutex> lock(mtx);

    // Espera até ser a vez do jogador ou o jogo acabar
    cv.wait(lock, [&](){ return player == current_player || game_over; });

    if(game_over) return false;
    if(board[row][col] != ' ') return false; // Jogada inválida

    board[row][col] = player; // Marca a jogada
    display_board();

    game_over = is_game_over();
    if(!game_over){
      current_player = (player == 'O') ? 'X' : 'O'; // Alterna jogador
    }

    cv.notify_all(); // Notifica o outro jogador
    return true;
  }
  
  bool check_win(char player) {
    // Verificar se o jogador atual venceu o jogo
    // linhas
    for(int i = 0; i < 3; i++){
      if(player == board[i][0] && player == board[i][1] && player == board[i][2]){
        winner = player;
        return 1;
      }
    }
    // colunas
    for(int i = 0; i < 3; i++){
      if(player == board[0][i] && player == board[1][i] && player == board[2][i]){
        winner = player;
        return 1;
      }
    }
    // diagonais
    if(player == board[0][0] && player == board[1][1] && player == board[2][2]){
      winner = player;
      return 1;
    }
    if(player == board[0][2] && player == board[1][1] && player == board[2][0]){
      winner = player;
      return 1;
    }
    return 0;
  }
  
  bool check_draw() {
    // Verificar se houve um empate
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(board[i][j] == ' '){
          return 0;
        }
      }
    }
    return 1;
  }
  
  bool is_game_over() {
    // Retornar se o jogo terminou
    if(check_win('X') || check_win('O')){
      return 1;
    }else if(check_draw()){
      winner = 'D';
      return 1;
    }else{
      return 0;
    }
  }
  
  char get_winner() {
    // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
    return winner;
  }
};

// Classe Player
class Player {
  private:
  TicTacToe& game; // Referência para a instância do jogo
  char symbol; // Símbolo do jogador ('X' ou 'O')
  std::string strategy; // Estratégia do jogador
  
  public:
  Player(TicTacToe& g, char s, std::string strat) 
  : game(g), symbol(s), strategy(strat) {}
  
  void play() {
    // Executar jogadas de acordo com a estratégia escolhida
    while(game.get_winner() == '-'){
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      if(strategy == "sequential"){
        play_sequential();
      }else{
        play_random();
      }
    }
  }
  
  private:
  void play_sequential() {
    // Implementar a estratégia sequencial de jogadas
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(game.make_move(symbol, i, j)){
          return;
        }
      }
    }
  }
  
  void play_random() {
    // Implementar a estratégia aleatória de jogadas
    int l;
    int c;
    bool fim = 0;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distr(0, 2);
    while(!fim && game.get_winner() == '-'){
      l = distr(gen);
      c = distr(gen);
      fim = game.make_move(symbol, l, c);
    } 
  }
};

// Retorna data/hora formatada para exibir nos logs
string now_str() {
    auto t = chrono::system_clock::to_time_t(chrono::system_clock::now());
    ostringstream ss;
    ss << put_time(localtime(&t), "%d/%m/%Y %H:%M:%S");
    return ss.str();
}




// Função principal
int main() {
  // Inicializar o jogo e os jogadores
  TicTacToe tabuleiro;
  tabuleiro.display_board();
  Player X(tabuleiro, 'X', "sequential");
  Player O(tabuleiro, 'O', "random");
  
  // Criar as threads para os jogadores
  std::thread Jogador1(&Player::play, &X);
  std::thread Jogador2(&Player::play, &O);
  
  // Aguardar o término das threads
  Jogador1.join();
  Jogador2.join();
  
  // Exibir o resultado final do jogo
  char vencedor = tabuleiro.get_winner();
  if(vencedor == 'D'){
    std::cout<<" Empate!\n";
  }else{
    std::cout<<" Vencedor: "<<vencedor<<"\n";
  }
  std::cout << "Data e hora da execução: " << now_str() << "\n";
  return 0;
}
