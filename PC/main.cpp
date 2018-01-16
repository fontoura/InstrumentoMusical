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

#include <allegro.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <string>
#include <sstream>
#include <cmath>
#include "instrumento.h"

#define LARGURA_TELA 640
#define ALTURA_TELA 480

#define LARGURA_BRACO 500
#define ALTURA_CORDA 50

// função de início do allegro.
inline void init() {
	allegro_init();

	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, LARGURA_TELA, ALTURA_TELA, 0, 0);

	install_timer();
	install_keyboard();
	install_mouse();
    show_os_cursor(MOUSE_CURSOR_ARROW);
}

// função de término do allegro.
inline void deinit() {
	clear_keybuf();
	allegro_exit();
}

// função auxiliar, de espera.
void sleep(int n) {
    long int t = clock() + (n*1000)/CLOCKS_PER_SEC;
    while (clock() < t);
}


int main() {
	init();

    BITMAP* bmp_buffer = create_bitmap(SCREEN_W, SCREEN_H);
    bool repetir = true;
    bool abortar = false;

    while (repetir) {
        repetir = false;
        abortar = false;
        unsigned int porta = 1;
        bool continuar = true;
        while (continuar) {
            clear_bitmap(bmp_buffer);
            char mensagem[64];
            sprintf(mensagem, "Selecione a porta COM (+/-): COM %d.", porta);
            textout_centre_ex(bmp_buffer, font, mensagem, SCREEN_W / 2, SCREEN_H / 4, makecol(255, 255, 255), 0);
            textout_centre_ex(bmp_buffer, font, "Pressione ENTER para avancar", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), 0);
            textout_centre_ex(bmp_buffer, font, "Pressione ESC para cancelar.", SCREEN_W / 2, (3 * SCREEN_H) / 4, makecol(255, 255, 255), 0);
            blit(bmp_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            if (key[KEY_PLUS_PAD]) if (porta < 12) {
                sleep(200);
                porta ++;
            }
            if (key[KEY_MINUS_PAD]) if (porta > 1) {
                sleep(200);
                porta --;
            }
            if (key[KEY_ESC]) {
                continuar = false;
                abortar = true;
            }
            if (key[KEY_ENTER] || key[KEY_ENTER_PAD]) {
                continuar = false;
                abortar = false;
            }
        }

        if (!abortar) {
            clear_bitmap(bmp_buffer);
            textout_centre_ex(bmp_buffer, font, "Conectando-se ao instrumento.", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), 0);
            blit(bmp_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            char nome_porta[16];
            sprintf(nome_porta, "\\\\.\\COM%d", porta);
            ConexaoInstrumento instrumento(nome_porta, 500, 300);
            if (instrumento.conectado()) {
                sleep(2000);

                abortar = false;

                // lê os dados sobre as cordas
                unsigned int total_cordas = 0;
                while (!abortar && !instrumento.lerTotalCordas(&total_cordas))
                    if (key[KEY_ESC])
                        abortar = true;

                if (!abortar) {
                    // lê o total de casas.
                    unsigned int total_casas = 0;
                    while (!abortar && !instrumento.lerPrecisaoCordas(&total_casas))
                        if (key[KEY_ESC])
                            abortar = true;

                    if (!abortar) {
                        // prepara os dados para o sistema gráfico (tamanho dos trastes).
                        unsigned int posicao_trastes[total_casas];
                        for (unsigned int i = 0; i < total_casas; i ++)
                            posicao_trastes[i] = LARGURA_BRACO - (unsigned int)(LARGURA_BRACO * pow(2, -((double)i)/12));

                        // prepara os dados das cordas.
                        corda estado_cordas[total_cordas];
                        for (unsigned int i = 0; i < total_cordas; i ++) {
                            estado_cordas[i].posicao = 0;
                            estado_cordas[i].soando = false;
                        }

                        // prepara as variáveis de medição de tempo.
                        unsigned long long momento_inicial = clock();
                        unsigned long long tempo_total = 0;

                        // para as estatísticas de erro.
                        unsigned int acertos = 0;
                        unsigned int erros = 0;
                        unsigned int leituras = 0;

                        // prepara as cores de desenho
                        int cor_fundo = makecol(255, 255, 255);
                        int cor_corda = makecol(0, 0, 0);
                        int cor_traste = makecol(192, 192, 192);
                        int cor_dedo = makecol(255, 0, 0);

                        while (!abortar) {
                            // lê os estados das cordas.
                            leituras ++;
                            for (unsigned int i = 0; i < total_cordas; i ++) {
                                if (instrumento.lerCorda(i + 1, estado_cordas + i)) acertos ++;
                                else erros ++;
                            }

                            // desenha o braco da guitarra.
                            int x0 = (LARGURA_TELA - LARGURA_BRACO) / 2;
                            int y0 = (ALTURA_TELA - ALTURA_CORDA * total_cordas) / 2;
                            clear_bitmap(bmp_buffer);
                            rectfill(bmp_buffer, x0, y0, x0 + LARGURA_BRACO, y0 + ALTURA_CORDA * total_cordas, cor_fundo);
                            for (unsigned int i = 0; i < total_casas; i ++) {
                                rectfill(bmp_buffer, x0 + posicao_trastes[i], y0, x0 + posicao_trastes[i], y0 + ALTURA_CORDA * total_cordas, cor_traste);
                            }

                            // desenha as cordas da guitarra
                            for (unsigned int i = 0; i < total_cordas; i ++) {
                                rectfill(bmp_buffer, x0, y0 + ALTURA_CORDA * i + ALTURA_CORDA / 2 - 1, x0 + LARGURA_BRACO, y0 + ALTURA_CORDA * i + ALTURA_CORDA / 2 + 1, cor_corda);
                                if (estado_cordas[i].soando) {
                                    rectfill(bmp_buffer, x0 + posicao_trastes[estado_cordas[i].posicao] - 2, y0 + ALTURA_CORDA * i + ALTURA_CORDA / 2 - 1, x0 + LARGURA_BRACO, y0 + ALTURA_CORDA * i + ALTURA_CORDA / 2 + 1, cor_dedo);
                                }
                                rectfill(bmp_buffer, x0 + posicao_trastes[estado_cordas[i].posicao] - 2, y0 + ALTURA_CORDA * i + ALTURA_CORDA / 2 - 2, x0 + posicao_trastes[estado_cordas[i].posicao] + 2, y0 + ALTURA_CORDA * i + ALTURA_CORDA / 2 + 2, cor_dedo);
                            }

                            // escreve as estatísticas de velocidade sobre o braço
                            tempo_total = ((clock() - momento_inicial)*1000)/CLOCKS_PER_SEC;
                            char mensagem[64];
                            sprintf(mensagem, "Tempo decorrido: %d s. Frequencia: %d Hz. Taxa de erro: %d%c.", (int)(tempo_total/1000), (int)((1000*leituras)/tempo_total), (int)(erros/(acertos + erros)), '%');
                            textout_centre_ex(bmp_buffer, font, mensagem, SCREEN_W / 2, y0 / 2, makecol(255, 255, 255), 0);

                            textout_centre_ex(bmp_buffer, font, "Pressione ESC para sair.", SCREEN_W / 2, SCREEN_H - y0 / 2, makecol(255, 255, 255), 0);


                            // escreve a string com os erros e os tempos
                            abortar = key[KEY_ESC];
                            blit(bmp_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
                        }
                    }
                }
            } else {
                clear_bitmap(bmp_buffer);
                textout_centre_ex(bmp_buffer, font, "Nao foi possivel conectar-se ao instrumento.", SCREEN_W / 2, SCREEN_H / 3, makecol(255, 255, 255), 0);
                textout_centre_ex(bmp_buffer, font, "Pressione ENTER para continuar.", SCREEN_W / 2, (2 * SCREEN_H) / 3, makecol(255, 255, 255), 0);
                blit(bmp_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
                while (key[KEY_ENTER] || key[KEY_ENTER_PAD]);
                while (!key[KEY_ENTER] && !key[KEY_ENTER_PAD]);
                while (key[KEY_ENTER] || key[KEY_ENTER_PAD]);
            }
            clear_bitmap(bmp_buffer);
            textout_centre_ex(bmp_buffer, font, "Deseja tentar novamente (s/n)?", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), 0);
            blit(bmp_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            bool continuar = true;
            while (continuar) {
                if (key[KEY_S]) {
                    repetir = true;
                    continuar = false;
                }
                if (key[KEY_N]) {
                    repetir = false;
                    continuar = false;
                }
            }
        }
    }
    clear_bitmap(bmp_buffer);
    textout_centre_ex(bmp_buffer, font, "Ate mais!", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), 0);
    blit(bmp_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    sleep(2000);
	deinit();
	return 0;
}
END_OF_MAIN()
