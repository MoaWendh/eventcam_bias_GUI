#ifndef MENUGUI_HPP
#define MENUGUI_HPP

#include <QLineEdit>
#include <map>
#include <string>

// Esta função inicializa e exibe uma interface gráfica simples com dois botões:
// "Visualizar eventos" — para iniciar a visualização dos eventos da câmera de eventos,
// "Sair" — para encerrar a aplicação.
//
// A lógica da GUI e os callbacks estão implementados em menuGUI.cpp.
// A função pode ser chamada diretamente a partir da função main().


// Struct para guardar os bias:
struct BiasFields_RD {
    QLineEdit *diff;
    QLineEdit *diff_on;
    QLineEdit *diff_off;
    QLineEdit *fo;
    QLineEdit *hpf;
    QLineEdit *refr;
};

struct BiasFields_WR {
    QLineEdit *diff_on;
    QLineEdit *diff_off;
    QLineEdit *fo;
    QLineEdit *hpf;
};

void launchMenuGUI(int argc, char *argv[]);

void ler_biases(BiasFields_RD &camposRD, BiasFields_WR &camposWR);
bool escrever_biases_cam(const BiasFields_WR &campos_bias);
void salvar_biases_em_json(BiasFields_RD &camposRD, BiasFields_WR &camposWR);

#endif // MENUGUI_HPP
