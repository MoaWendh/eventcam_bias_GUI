// Moacir
// Para testar as configurações dos biases.
// Data: 17/07/2025


#include <metavision/sdk/driver/camera.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>

#include "eventoViewer.hpp"
#include "menuGUI.hpp"
#include "camera_global.hpp"  // Define o escopo da câmera para os demais arquivos

// Instancia um objeto cam com escopo global:
//Metavision::Camera cam;


int main(int argc, char *argv[]) {
    // Só para limpar o terminal:
    limpar_terminal();

    /*
    // Cria um objeto "config", definido no SDK Metavision, que será um "container" para os parâmetros de configuração da câmera:
    Metavision::DeviceConfig config;

    // Atenção!!!!
    // Para setar os valores dos biases, para aversão SDK 4.6 é necessaŕio usa a funçao "config.enable_biases_range_check_bypass(true)".
    // Cuidado, não usar valores fora da faixa permitida para os sensor. Verificar os valores em:
    // https://docs.prophesee.ai/stable/hw/manuals/biases.html    
    config.enable_biases_range_check_bypass(true);
    

    // Dependendo do argumento passado em vez de abrir a câmera, será aberto um arquivo pré-gravado
    // ou seja, um arquivo de eventos anteriormente registrados:
    // A chamad do program sem argumentos abrirá o device da câmera
    if (argc >= 2 && argv[1] != nullptr)  {
        // if we passed a file path, open it
        // Passanod o arquivo bruto "monitoring_40_50hz.raw" ao chamar o executável "eventCam":
        // por exempĺo: ./eventCam monitoring_40_50hz.raw
        cam = Metavision::Camera::from_file(argv[1]);
    } else {
        // Abre a primeira cãmera disponível na USB, e já passa a configuração através da variável "config":
        try {
            cam = Metavision::Camera::from_first_available(config);
        } 
        catch (const std::exception &e) {
            std::cerr << "Erro ao abrir a câmera: " << e.what() << std::endl;
            return false;
        }
    }*/

    // Chama a interface GUI para interação com o usuário.
    launchMenuGUI(argc, argv);

}