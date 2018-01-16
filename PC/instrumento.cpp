/*
Copyright (c) 2009, Felipe Luiz Bill, Felipe Michels Fontoura, Leandro Piekarski do Nascimento and
Lucio Eiji Assaoka Hossaka
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list
      of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this
      list of conditions and the following disclaimer in the documentation and/or
      other materials provided with the distribution.
    * Neither the name of Universidade Federal do Paraná nor the names of its contributors
      may be used to endorse or promote products derived from this software without specific
      prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <time.h>
#include "instrumento.h"

using namespace std;

#define MAXIMO_TENTATIVAS 64

char* CONST_INSTRUCOES[2] = {"set.information", "set.status"};
#define INSTRUCOES_TOTAL 2
#define INSTRUCAO_SET_INFORMATION 0
#define INSTRUCAO_SET_STATUS 1

char* CONST_PARAMETROS[6] = {"key", "of", "index", "pressed", "sounding", "value"};
#define PARAMETROS_ALFA_TOTAL 2
#define PARAMETROS_NUM_TOTAL 4
#define PARAMETROS_TOTAL (PARAMETROS_ALFA_TOTAL + PARAMETROS_NUM_TOTAL)
#define PARAMETRO_KEY 0
#define PARAMETRO_OF 1
#define PARAMETRO_INDEX 2
#define PARAMETRO_PRESSED 3
#define PARAMETRO_SOUNDING 4
#define PARAMETRO_VALUE 5

char* CONST_VALORES[3] = {"string", "string count", "position range"};
#define VALORES_TOTAL 3
#define VALOR_STRING 0
#define VALOR_STRING_COUNT 1
#define VALOR_POSITION_RANGE 2

// cria uma conexão com o instrumento.
ConexaoInstrumento::ConexaoInstrumento(char* porta, unsigned int esperaMaxima_1char, unsigned int esperaMaxima_total) {
    this->handle = INVALID_HANDLE_VALUE;
    this->esperaMaxima_1char = esperaMaxima_1char;
    this->esperaMaxima_total = esperaMaxima_total;
    DCB parametrosSerialDBC = {0};
    COMMTIMEOUTS esperas = {0};

    // abre a conexão serial.
    this->handle = CreateFile(porta,
                              GENERIC_READ | GENERIC_WRITE,
                              0, 0,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

    // se a conexão serial não foi criada, notifica um erro.
    if (this->handle == INVALID_HANDLE_VALUE) goto erro;

    // define os parâmetros da conexão.
    if (!GetCommState(this->handle, &parametrosSerialDBC)) goto erro;
    parametrosSerialDBC.BaudRate = CBR_115200;
    parametrosSerialDBC.ByteSize = 8;
    parametrosSerialDBC.StopBits = ONESTOPBIT;
    parametrosSerialDBC.Parity = NOPARITY;
    if (!SetCommState(handle, &parametrosSerialDBC)) goto erro;

    // define os tempos de espera.
    if (!GetCommTimeouts(this->handle, &esperas)) goto erro;
    esperas.ReadIntervalTimeout = 50;
    esperas.ReadTotalTimeoutConstant = 50;
    esperas.ReadTotalTimeoutMultiplier = 10;
    esperas.WriteTotalTimeoutConstant = 50;
    esperas.WriteTotalTimeoutMultiplier = 10;
    if(!SetCommTimeouts(this->handle, &esperas)) goto erro;

    goto sucesso;

    erro:
    CloseHandle(this->handle);
    this->handle = INVALID_HANDLE_VALUE;

    sucesso:
    return;
};

// fecha a conexão com o instrumento.
ConexaoInstrumento::~ConexaoInstrumento() {
    if (this->handle != INVALID_HANDLE_VALUE) {
        CloseHandle(this->handle);
    }
};

// define os tempos de espera.
void ConexaoInstrumento::definirEsperas(unsigned int esperaMaxima_1char, unsigned int esperaMaxima_total) {
    this->esperaMaxima_1char = esperaMaxima_1char;
    this->esperaMaxima_total = esperaMaxima_total;
};

// recebe uma string.
string* ConexaoInstrumento::receberString() {
    if (this->handle == INVALID_HANDLE_VALUE) return NULL;

    unsigned long int tempo = clock();
    unsigned long int tempoMax = tempo + (esperaMaxima_1char * CLOCKS_PER_SEC) / 1000;

    std::stringstream fluxo(std::stringstream::out);
    char entrada;
    DWORD leitura = 0;

    while (leitura == 0 && tempo < tempoMax) {
        ReadFile(this->handle, &entrada, 1, &leitura, NULL);
        tempo = clock();
    }

    if (tempo > tempoMax) return NULL;
    tempoMax = clock() + (esperaMaxima_total * CLOCKS_PER_SEC) / 1000;

    while (leitura == 0 && (entrada == 10 || entrada == 13) && tempo < tempoMax) {
        ReadFile(this->handle, &entrada, 1, &leitura, NULL);
        tempo = clock();
    }

    while (entrada != 10 && entrada != 13 && tempo < tempoMax) {
        if (leitura) fluxo << entrada;
        ReadFile(this->handle, &entrada, 1, &leitura, NULL);
        tempo = clock();
    }

    if (tempo > tempoMax) return NULL;

    return new string(fluxo.str());
};

// envia uma string.
bool ConexaoInstrumento::enviarString(string* str) {
    if (this->handle == INVALID_HANDLE_VALUE) return false;

    DWORD enviados = 0;
    char pulaLinha[3] = {13, 10, 0};

    WriteFile(handle, pulaLinha, 2, &enviados, NULL);
    WriteFile(handle, str->c_str(), str->size(), &enviados, NULL);
    WriteFile(handle, pulaLinha, 2, &enviados, NULL);

    return true;
}

// descarta todos os dados recebidos.
void ConexaoInstrumento::descartarDados() {
    if (this->handle != INVALID_HANDLE_VALUE)
        FlushFileBuffers(this->handle);
};

// verifica se está conectado à porta serial.
bool ConexaoInstrumento::conectado() {
    return (handle != INVALID_HANDLE_VALUE);
};

// termina a leitura de dados do computador.
void ConexaoInstrumento::terminarLeitura(stringstream** fluxo) {
  if (*fluxo) delete (*fluxo);
  *fluxo = NULL;
};

// verifica se a leitura de dados do computador está ativa.
bool ConexaoInstrumento::leituraAtiva(stringstream** fluxo) {
  if (*fluxo) return !(**fluxo).eof();
  else return false;
};

// lê um caractere.
bool ConexaoInstrumento::lerCaractere(stringstream** fluxo, char* caractere) {
  if (*fluxo) if (!(**fluxo).eof()) {
    (**fluxo) >> noskipws >> *caractere;
    return true;
  }
  return false;
};

// pula espaços em branco.
bool ConexaoInstrumento::pularEspacos(stringstream** fluxo, char* caractere) {
  if (lerCaractere(fluxo, caractere)) {
    while (*caractere == ' ')
      if (!lerCaractere(fluxo, caractere))
        return false;
    return true;
  } else return false;
};

bool ConexaoInstrumento::lerInstrucao(stringstream** fluxo, char** instrucoes, int numInstrucoes, unsigned int* retorno) {
  char caractere;
  if (this->pularEspacos(fluxo, &caractere)) {
    if (caractere == 10 || caractere == 13) terminarLeitura(fluxo);
    // compara caractere a caractere com todas as instruções.
    bool resultado[numInstrucoes];
    for (int i = 0; i < numInstrucoes; i ++)
      resultado[i] = true;
    int indice = 0;
    while (caractere != ' ') {
      for (int i = 0; i < numInstrucoes; i ++) {
        if (resultado[i]) {
          if (instrucoes[i][indice] == 0) resultado[i] = false;
          else resultado[i] = instrucoes[i][indice] == caractere;
        }
      }
      indice ++;
      if (!lerCaractere(fluxo, &caractere)) caractere = ' ';
    }
    for (int i = 0; i < numInstrucoes; i ++) {
      if (resultado[i]) {
        if (instrucoes[i][indice] == 0) {
          *retorno = i;
          return true;
        }
      }
    }
  }
  return false;
}

bool ConexaoInstrumento::lerParametro(stringstream** fluxo, char** parametros, unsigned int numParametrosAlfa, unsigned int numParametrosNum, unsigned int* retorno, char** valores, unsigned int numValores, unsigned int* valor) {
  char caractere;
  int numParametros = numParametrosAlfa + numParametrosNum;
  if (this->pularEspacos(fluxo, &caractere)) {
    if (caractere == 10 || caractere == 13) terminarLeitura(fluxo);
    // compara caractere a caractere com todas as instruções.
    bool resultado[numParametros];
    for (int i = 0; i < numParametros; i ++)
      resultado[i] = true;
    int indice = 0;
    while (caractere != ' ' && caractere != '=') {
      for (int i = 0; i < numParametros; i ++) {
        if (resultado[i]) {
          if (parametros[i][indice] == 0) resultado[i] = false;
          else resultado[i] = parametros[i][indice] == caractere;
        }
      }
      indice ++;
      if (!lerCaractere(fluxo, &caractere)) caractere = ' ';
    }
    // pula os espaços e procura pelo sinal de igual.
    if (caractere == ' ') if (! this->pularEspacos(fluxo, &caractere)) {
      this->terminarLeitura(fluxo);
      return false;
    }
    if (caractere != '=')  {
      this->terminarLeitura(fluxo);
      return false;
    }
    if (! this->pularEspacos(fluxo, &caractere))  {
      this->terminarLeitura(fluxo);
      return false;
    }
    if (caractere != '"')  {
      this->terminarLeitura(fluxo);
      return false;
    }
    if (! this->lerCaractere(fluxo, &caractere))  {
      this->terminarLeitura(fluxo);
      return false;
    }
    bool alfa = true;
    for (int i = 0; i < numParametros; i ++) {
      if (resultado[i]) {
        if (parametros[i][indice] == 0) {
          if (i >= numParametrosAlfa) alfa = false;
        }
      }
    }
    if (alfa) {
      // compara caractere a caractere com os valores.
      bool comparacao[numValores];
      for (int i = 0; i < numValores; i ++)
        comparacao[i] = true;
      int indice2 = 0;
      while (caractere != '"') {
        if (caractere == '\\')
          lerCaractere(fluxo, &caractere);
        for (int i = 0; i < numValores; i ++) {
          if (comparacao[i]) {
            if (valores[i][indice2] == 0) comparacao[i] = false;
            else comparacao[i] = valores[i][indice2] == caractere;
          }
        }
        indice2 ++;
        if (! lerCaractere(fluxo, &caractere)) caractere = '"';
      }

      for (int i = 0; i < numParametros; i ++) {
            if (resultado[i]) {
              if (parametros[i][indice] == 0) {
                // procura o valor
                for (int j = 0; j < numValores; j ++) {
                  if (comparacao[j]) {
                    if (valores[j][indice2] == 0) {
                      *retorno = i;
                      *valor = j;
                      return true;
                    }
                  }
                }
                return false;
              }
            }
      }
    } else {
      *valor = 0;
      bool numero = true;
      while (caractere != '"') {

        if (caractere == '\\')
          this->lerCaractere(fluxo, &caractere);
        if (numero) {
          if (caractere >= 'a' && caractere <= 'f') *valor = *valor * 16 + 10 + caractere - 'a';
          else if (caractere >= 'A' && caractere <= 'F') *valor = *valor * 16 + 10 + caractere - 'A';
          else if (caractere >= '0' && caractere <= '9') *valor = *valor * 16 + caractere - '0';
          else numero = false;
        }
        if (! this->lerCaractere(fluxo, &caractere)) caractere = '"';
      }
      for (int i = 0; i < numParametros; i ++) {
        if (resultado[i]) {
          if (parametros[i][indice] == 0) {
            *retorno = i;
            return true;
          }
        }
      }
    cout << "[n2]";
    }
  }
  return false;
}

bool ConexaoInstrumento::lerCorda(int numero, corda* destino) {
    bool continuar = true;
    bool sucesso = false;
    int tentativas = 0;
    descartarDados();
    string enviar (" get.status of = \"string\" index = \"");
    enviar += '0' + numero;
    enviar += "\" ";
    while (continuar) {
        descartarDados();
        enviarString(&enviar);
        string* recebida = this->receberString();
        stringstream* fluxo = new std::stringstream(*recebida, stringstream::in);
        unsigned int instrucao, parametro, valor;
        unsigned int valor_parametro[PARAMETROS_TOTAL];
        bool parametro_recebido[PARAMETROS_TOTAL];
        if (lerInstrucao(&fluxo, CONST_INSTRUCOES, INSTRUCOES_TOTAL, &instrucao)) {
            if (instrucao == INSTRUCAO_SET_STATUS) {
                sucesso = true;
                for (int i = 0; i < PARAMETROS_TOTAL; i ++) {
                    valor_parametro[i] = 0;
                    parametro_recebido[i] = false;
                }
                while (leituraAtiva(&fluxo)) if (lerParametro(&fluxo, CONST_PARAMETROS, PARAMETROS_ALFA_TOTAL, PARAMETROS_NUM_TOTAL, &parametro, CONST_VALORES, VALORES_TOTAL, &valor)) {
                    parametro_recebido[parametro] = true;
                    valor_parametro[parametro] = valor;
                }
                if (parametro_recebido[PARAMETRO_INDEX] && parametro_recebido[PARAMETRO_OF] &&
                    parametro_recebido[PARAMETRO_PRESSED] && parametro_recebido[PARAMETRO_SOUNDING])
                if (valor_parametro[PARAMETRO_INDEX] == numero) {
                    destino->posicao = valor_parametro[PARAMETRO_PRESSED];
                    destino->soando = valor_parametro[PARAMETRO_SOUNDING];
                }

            }
        }
        tentativas ++;
        delete recebida;
        if (fluxo) delete fluxo;
        if (tentativas > MAXIMO_TENTATIVAS) continuar = false;
        if (sucesso) continuar = false;
    };
    return sucesso;
};


bool ConexaoInstrumento::lerTotalCordas(unsigned int* destino) {
    bool continuar = true;
    bool sucesso = false;
    int tentativas = 0;
    descartarDados();
    string enviar (" get.information key = \"string count\" ");
    while (continuar) {
        descartarDados();
        enviarString(&enviar);
        string* recebida = this->receberString();
        stringstream* fluxo = new std::stringstream(*recebida, stringstream::in);
        unsigned int instrucao, parametro, valor;
        unsigned int valor_parametro[PARAMETROS_TOTAL];
        bool parametro_recebido[PARAMETROS_TOTAL];
        if (lerInstrucao(&fluxo, CONST_INSTRUCOES, INSTRUCOES_TOTAL, &instrucao)) {
            if (instrucao == INSTRUCAO_SET_INFORMATION) {
                sucesso = true;
                for (int i = 0; i < PARAMETROS_TOTAL; i ++) {
                    valor_parametro[i] = 0;
                    parametro_recebido[i] = false;
                }
                while (leituraAtiva(&fluxo)) if (lerParametro(&fluxo, CONST_PARAMETROS, PARAMETROS_ALFA_TOTAL, PARAMETROS_NUM_TOTAL, &parametro, CONST_VALORES, VALORES_TOTAL, &valor)) {
                    parametro_recebido[parametro] = true;
                    valor_parametro[parametro] = valor;
                }
                if (parametro_recebido[PARAMETRO_KEY] && parametro_recebido[PARAMETRO_VALUE])
                if (valor_parametro[PARAMETRO_KEY] == VALOR_STRING_COUNT) {
                    *destino = valor_parametro[PARAMETRO_VALUE];
                }

            }
        }
        tentativas ++;
        delete recebida;
        if (fluxo) delete fluxo;
        if (tentativas > MAXIMO_TENTATIVAS) continuar = false;
        if (sucesso) continuar = false;
    };
    return sucesso;
};

