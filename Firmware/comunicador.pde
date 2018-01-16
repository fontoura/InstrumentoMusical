#include <stdlib.h>

#define TOTAL_INSTRUCOES 2
char* instrucoes[TOTAL_INSTRUCOES] = {"get.information", "get.status"};
#define INSTRUCAO_GET_INFORMATION 0
#define INSTRUCAO_GET_STATUS 1

#define TOTAL_PARAMETROS_ALFA 2
#define TOTAL_PARAMETROS_NUM 1
#define TOTAL_PARAMETROS (TOTAL_PARAMETROS_ALFA + TOTAL_PARAMETROS_NUM)
char* parametros[TOTAL_PARAMETROS] = {"key", "of", "index"};
#define PARAMETRO_KEY 0
#define PARAMETRO_OF 1
#define PARAMETRO_INDEX 2

#define TOTAL_VALORES 3
char* valores[TOTAL_VALORES] = {"string", "string count", "position range"};
#define VALOR_STRING 0
#define VALOR_STRING_COUNT 1
#define VALOR_POSITION_RANGE 2

class ConexaoComputador {
  private:
    unsigned long int tempo;
    unsigned long int tempoMaximo;
    unsigned int esperaMaxima;
    bool lendo;
  
  public:
    
    ConexaoComputador(unsigned int esperaMaxima);
    
    void comecarLeitura();
    void terminarLeitura();
    bool leituraAtiva();
    bool lerCaractere(char* caractere);
    
    bool pularEspacos(char* caractere);
    void pularLinha();
    
    bool lerInstrucao(char** instrucoes, int numInstrucoes, unsigned int* resultado);
    bool lerParametro(char** parametros, unsigned int numParametrosAlfa, unsigned int numParametrosNum, unsigned int* retorno, char** valores, unsigned int numValores, unsigned int* valor);

    void escreverCorda(unsigned int corda);
};

ConexaoComputador conexao(200);
unsigned int valor_parametro[TOTAL_PARAMETROS];
bool parametro_recebido[TOTAL_PARAMETROS];

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) {
    conexao.comecarLeitura();
    
    unsigned int parametro, instrucao, valor;
    if (conexao.lerInstrucao(instrucoes, TOTAL_INSTRUCOES, &instrucao)) {

      for (int i = 0; i < TOTAL_PARAMETROS; i ++) {
        valor_parametro[i] = 0;
        parametro_recebido[i] = false;
      }
      while (conexao.leituraAtiva()) if (conexao.lerParametro(parametros, TOTAL_PARAMETROS_ALFA, TOTAL_PARAMETROS_NUM, &parametro, valores, TOTAL_VALORES, &valor)) {
        parametro_recebido[parametro] = true;
        valor_parametro[parametro] = valor;
      }
      
      if (instrucao == INSTRUCAO_GET_STATUS) {
        if (parametro_recebido[PARAMETRO_INDEX] && parametro_recebido[PARAMETRO_OF]) {
          if (valor_parametro[PARAMETRO_OF] == VALOR_STRING) {
            if (valor_parametro[PARAMETRO_INDEX] == 1) {
              conexao.escreverCorda(1);
            } else if (valor_parametro[PARAMETRO_INDEX] == 2) {
              conexao.escreverCorda(2);
            } else {
              Serial.println("raise.error kind=\"wrong string index\"");
            }
          } else {
            Serial.println("raise.error kind=\"wrong entity name\"");
          }
        } else {
          Serial.println("raise.error kind=\"missing parameter\"");
        }
      }
      
      
      if (instrucao == INSTRUCAO_GET_INFORMATION) {
        if (parametro_recebido[PARAMETRO_KEY]) {
          if (valor_parametro[PARAMETRO_KEY] == VALOR_STRING_COUNT) {
            Serial.println("set.information key=\"string count\" value=\"2\"");
          } else if (valor_parametro[PARAMETRO_KEY] == VALOR_POSITION_RANGE) {
            Serial.println("set.information key=\"position range\" value=\"12\"");
          } else {
            Serial.println("raise.error kind=\"wrong entity name\"");
          }
        } else {
          Serial.println("raise.error kind=\"missing parameter\"");
        }
      }
    }
    conexao.pularLinha();
  }
  //delay(10);
}

