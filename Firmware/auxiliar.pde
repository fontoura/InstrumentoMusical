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

// cria uma nova conexão com o computador.
ConexaoComputador::ConexaoComputador(unsigned int esperaMax) {
    this->esperaMaxima = esperaMax;
    this->lendo = false;
};

// começa a leitura de dados do computador.
void ConexaoComputador::comecarLeitura() {
  this->lendo = true;
  this->tempo = millis();
  this->tempoMaximo = this->tempo + this->esperaMaxima;
};

// termina a leitura de dados do computador.
void ConexaoComputador::terminarLeitura() {
  this->lendo = false;
};

// verifica se a leitura de dados do computador está ativa.
bool ConexaoComputador::leituraAtiva() {
  return this->lendo;
};

// lê um caractere.
bool ConexaoComputador::lerCaractere(char* caractere) {
  if (this->lendo) {
    while (Serial.available() == 0) {
      delay(1);
      this->tempo = millis();
      if (tempo > tempoMaximo) {
        lendo = false;
        return false;
      }
    }
    *caractere = Serial.read();
    return true;
  } else return false;
};

// pula espaços em branco.
bool ConexaoComputador::pularEspacos(char* caractere) {
  if (lerCaractere(caractere)) {
    while (*caractere == ' ')
      if (!lerCaractere(caractere))
        return false;
    return true;
  } else return false;
};

// pula puladas de linha.
void ConexaoComputador::pularLinha() {
  char caractere;
  if (lerCaractere(&caractere)) {
    while (caractere != 13 && caractere != 10)
      if (!lerCaractere(&caractere)) return;
    this->terminarLeitura();
  }
};

bool ConexaoComputador::lerInstrucao(char** instrucoes, int numInstrucoes, unsigned int* retorno) {
  char caractere;
  if (this->pularEspacos(&caractere)) {
    if (caractere == 10 || caractere == 13) terminarLeitura();
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
      if (!lerCaractere(&caractere)) caractere = ' ';
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

bool ConexaoComputador::lerParametro(char** parametros, unsigned int numParametrosAlfa, unsigned int numParametrosNum, unsigned int* retorno, char** valores, unsigned int numValores, unsigned int* valor) {
  char caractere;
  int numParametros = numParametrosAlfa + numParametrosNum;
  if (this->pularEspacos(&caractere)) {
    if (caractere == 10 || caractere == 13) terminarLeitura();
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
      if (!lerCaractere(&caractere)) caractere = ' ';
    }
    
    // pula os espaços e procura pelo sinal de igual.
    if (caractere == ' ') if (! this->pularEspacos(&caractere)) {
      this->pularLinha();
      return false;
    }
    if (caractere != '=')  {
      this->pularLinha();
      return false;
    }
    if (! this->pularEspacos(&caractere))  {
      this->pularLinha();
      return false;
    }
    if (caractere != '"')  {
      this->pularLinha();
      return false;
    }
    if (! this->lerCaractere(&caractere))  {
      this->pularLinha();
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
          lerCaractere(&caractere);
        for (int i = 0; i < numValores; i ++) {
          if (comparacao[i]) {
            if (valores[i][indice2] == 0) comparacao[i] = false;
            else comparacao[i] = valores[i][indice2] == caractere;
          }
        }
        indice2 ++;
        if (! lerCaractere(&caractere)) caractere = '"';
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
          this->lerCaractere(&caractere);
        if (numero) {
          if (caractere >= 'a' && caractere <= 'f') *valor = *valor * 16 + 10 + caractere - 'a';
          else if (caractere >= 'A' && caractere <= 'F') *valor = *valor * 16 + 10 + caractere - 'A';
          else if (caractere >= '0' && caractere <= '9') *valor = *valor * 16 + caractere - '0';
          else numero = false;
        }
        if (! this->lerCaractere(&caractere)) caractere = '"';
      }
      
      for (int i = 0; i < numParametros; i ++) {
        if (resultado[i]) {
          if (parametros[i][indice] == 0) {
            *retorno = i;
            return true;
          }
        }
      }
    }
  }
  return false;
}

// escreve na saída serial o estado de uma das cordas.
void ConexaoComputador::escreverCorda(unsigned int corda) {
  Serial.print("set.status of=\"string\" index=\"");
  Serial.print(corda);
  Serial.println("\" pressed=\"5\" sounding=\"F\"");
};
