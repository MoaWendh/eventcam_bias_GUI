#ifndef FUNCOES_EVENTOS_HPP
#define FUNCOES_EVENTOS_HPP

#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/driver/camera.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <QLineEdit>

#include "menuGUI.hpp"

// Classe dedicada para processar eventos CD (Change Detection)
class EventAnalyzer {
public:
    // Variáveis de controle e análise
    int callback_counter = 0;
    int global_counter = 0;
    Metavision::timestamp global_max_t = 0;

    // Função chamada como callback para processar os eventos
    void analyze_events(const Metavision::EventCD *begin, const Metavision::EventCD *end);
};


struct BiasConfig {
    int diff= 0;
    int diff_on= 0;
    int diff_off= 0;
    int fo= 0;
    int hpf= 0;
    int refr= 0;
    int pr= 0; // somente leitura
};


void count_events(const Metavision::EventCD *begin, const Metavision::EventCD *end);

void frameEventView(int argc, char *argv[]);

void executar_loop_while_view(Metavision::Camera &cam, Metavision::Window &window, std::atomic<bool> &running);

void limpar_terminal();

#endif // FUNCOES_EVENTOS_HPP