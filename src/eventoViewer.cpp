// Moacir
// Para testar as configurações dos biases.
// Data: 17/07/2025



#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>
#include <metavision/hal/facilities/i_ll_biases.h>

#include <iostream>      
#include <map>
#include <string>
#include <exception>
#include <thread>

#include "eventoViewer.hpp"
#include "menuGUI.hpp"
#include "camera_global.hpp"   



// 
extern std::atomic<bool> running;


// Funcão simples para limpar o terminal:
void limpar_terminal() {
    std::cout << "\033[2J\033[H" << std::flush;
}

// Esta função é executada ocorre a chamada da callback do botão 'Visualizar eventos':
// Esta função executa o pipeline completo da camera de eventos:
// 1- Instancia a camera;
// 2- Instancia o objeto analizsador de eventos;
// 3- chama o cam.start();
// 4- no final chama cam.stop(). Encerrando os processos e threads de forma segura.
// OU seja, ao final ela fecha e destroi o objeto criado, se quiser ver os eventos em outra função, 
// o objeto deverá ser criado novamente 
void frameEventView(int argc, char *argv[]){
    // Instancia um objeto cam com escopo global:
    Metavision::Camera cam;

    // Só para limpar o terminal:
    limpar_terminal();


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
            return;
        }
    }


    // Inicializa a câmera com try para garantir segurança na abertura da câmera:
    try {
        // Inicializa a câmera "cam" (já instanciada) com a função "cam.star()": 
        cam.start();  // inicia a câmera

        // Testa abertura ok:
        if (!cam.is_running()) {
            std::cerr << "Erro: a câmera não está rodando após o start()." << std::endl;
            return;
        }
        std::cout << "Câmera iniciada com sucesso." << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Exceção ao iniciar a câmera: " << e.what() << std::endl;
        return;
    }

    std::cout<<"Variável cam.is_running() ""fora"" do loop POS 1="<<cam.is_running()<<std::endl;

    //
    std::atomic<bool> running(true);
    
    std::cout<<"Variável cam.is_running() ""fora"" do loop POS 2="<<cam.is_running()<<std::endl;

    // Para visualizar os eventos, é necessário construir quadros (frames virtuais) e renderizá-los.
    // A construção dos quadros será realizada por um gerador de quadros que acumulará os eventos ao longo do tempo.
    // Parta isso, é necessário fornecer a resolução da câmera, que pode ser obtida a partir da instância da câmera
    int camera_width  = cam.geometry().width();
    int camera_height = cam.geometry().height();

    // Também é necessário efinir o tempo acumulado, duração que define o volume de eventos, e ambém um frame rate.
    // Valores pré=definidos:  20ms e 50fps:
    const std::uint32_t acc = 20000;
    double fps= 50;

    // Usa-se a função "Metavision::PeriodicFrameGenerationAlgorithm() para informar, configurar, o gerador de eventos
    // dos parâmetros pré-definidos:
    auto frame_gen = Metavision::PeriodicFrameGenerationAlgorithm(camera_width, camera_height, acc, fps);

    std::cout<<"Variável cam.is_running() ""fora"" do loop POS 3="<<cam.is_running()<<std::endl;

    // Define-se a callback que irá ser chamada para passar os eventos ao gerador de frames virtuais:  
    cam.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        frame_gen.process_events(begin, end);
    });

    // Para renderizar os frames virtuais, cria-se uma janela da classe "Window" pertencente ao módulo "UI":
    Metavision::Window window("Metavision SDK Get Started", camera_width, camera_height,
                              Metavision::BaseWindow::RenderMode::BGR);


    // Configura-se a callback para o fechamaento da janela quando as teclas "esc ou "Q" forem pressionadas:
    window.set_keyboard_callback(
        [&window, running_ptr = &running](Metavision::UIKeyEvent key, int scancode, Metavision::UIAction action, int mods) {
            if (action == Metavision::UIAction::RELEASE &&
                (key == Metavision::UIKeyEvent::KEY_ESCAPE || key == Metavision::UIKeyEvent::KEY_Q)) {
                    // Fecha a janela de exibição dos frames virtuais:
                    window.set_close_flag();
                    std::cout<<"Window.set_close_flag"<<std::endl;
                
                    // Sinaliza para encerrar a thread que está processando os eventos
                    // Serve como garantia para uam execução segura com a thread.
                    running_ptr->store(false);
            }
        });

    // Define-se a callback no gerador de frams virtuais para chamar o objeto Window e exibir os frames:
    frame_gen.set_output_callback([&](Metavision::timestamp, cv::Mat &frame) { 
        window.show(frame); 
    });

    //executar_loop_while_view(cam, window, running);

    std::thread viewer_thread([&cam, &window, &running]() {
        //frameEventView(); // Essa função NÃO deve usar widgets Qt
        executar_loop_while_view(cam, window, running);
            // Ao sair do loop while é chamada a função "cam.stop()"" para finalizar a cãmera:
       // cam.stop();
       // std::cout<< "Visualização de eventos encerrada!"<< std::endl;
    });

    //viewer_thread.detach(); // Executa paralelamente sem bloquear a GUI   
    viewer_thread.join(); // Executa paralelamente sem bloquear a GUI   
    //viewer_thread.joinable();

}


void executar_loop_while_view(Metavision::Camera &cam, Metavision::Window &window, std::atomic<bool> &running){
    int ct=0;
    int ct_erro= 0;
    // keep running until the camera is off, the recording is finished or the escape key was pressed
    while (!window.should_close()) {
        if (cam.is_running() &&  running.load()) {    
            // O teclado e o mouse são consultamos com uma espera de 20 ms para evitar o uso de 100% de um núcleo da CPU
            // o valor repassado para a janela, onde o callback associado à tecla Escape solicitará o fechamento da janela
            static constexpr std::int64_t kSleepPeriodMs = 100;
            // A função Metavision::EventLoop::poll_and_dispatch() apenas repassa o valor 20 milisegundos que corresponde ao 
            // tempo de espera do pooling, tempo que espera uma tecla ou mouse ser acionado: 
            Metavision::EventLoop::poll_and_dispatch(kSleepPeriodMs);
            ct++;
            //std::cout<<"Contador loop while="<<ct<<std::endl;  
        }     
        else {
            if (!running.load()){
                ct_erro++;
                std::cout<<"Variável running.load() ="<<running.load()<<std::endl;   
                return;
            }
            if (!cam.is_running()){
                ct_erro++;
                std::cout<<"Variável cam.is_running() ""dentro"" do loop="<<cam.is_running()<<std::endl;
                return;   
            }       
        }  
    }
    cam.stop();
    std::cout<< "Visualização de eventos encerrada!"<< std::endl;    
}

/*

// Função chamada para ler os biases da câmera:
std::map<std::string, int> ler_biases() {
    std::map<std::string, int> valores_bias;

    // Efetua a leitura dos biases na câmera de eventos:
    auto biases = cam.get_device().get_facility<Metavision::I_LL_Biases>();

    // Testa sea aleitura foi ok:
    if (!biases) {
        std::cerr << "Interface I_LL_Biases não disponível." << std::endl;
        return valores_bias;
    }

    /*
    std::cout<<"Valores antigos lidos da câmera:"<<std::endl;
    std::cout<< "Valor lido do bias_diff_ON=" << biases->get("bias_diff_on")<<std::endl;
    std::cout<< "Valor lido do bias_diff_OFF=" << biases->get("bias_diff_off")<<std::endl;
    std::cout<< "Valor lido do bias_FO=" << biases->get("bias_fo")<<std::endl;
    std::cout<< "Valor lido do bias_HPF=" << biases->get("bias_hpf")<<std::endl;
    std::cout<<std::endl;
    
    // Passa os valores lidos para uma variável :
    for (const auto &[nome, valor] : biases->get_all_biases()) {
        valores_bias[nome] = valor;
        //std::cout << nome << ": " << valor << std::endl;
    }

    return valores_bias;
}
*/

/*

// FUnção usada para escrever os valors dos biases na câmera:
bool escrever_biases_cam(const BiasFields_WR &campos_bias) {
   
    // efetua a leitura dos biases da camera:
    auto biases_a = cam.get_device().get_facility<Metavision::I_LL_Biases>();
    if (!biases_a) {
        std::cerr << "Interface I_LL_Biases não disponível." << std::endl;
        return false;
    }

    std::cout<<"Valores antigos lidos da câmera:"<<std::endl;
    std::cout<< "Valor lido do bias_diff_ON=" << biases_a->get("bias_diff_on")<<std::endl;
    std::cout<< "Valor lido do bias_diff_OFF=" << biases_a->get("bias_diff_off")<<std::endl;
    std::cout<< "Valor lido do bias_FO=" << biases_a->get("bias_fo")<<std::endl;
    std::cout<< "Valor lido do bias_HPF=" << biases_a->get("bias_hpf")<<std::endl;
    std::cout<<std::endl;
    

    // Mapa com nomes e valores extraídos dos campos
    std::map<std::string, int> mapa_bias = {
        {"bias_diff_on", campos_bias.diff_on->text().toInt()},
        {"bias_diff_off", campos_bias.diff_off->text().toInt()},
        {"bias_fo", campos_bias.fo->text().toInt()},
        {"bias_hpf", campos_bias.hpf->text().toInt()}
    };

    // Escrita na câmera
    for (const auto &[nome, valor] : mapa_bias) {
        try {
            if (!biases_a->set(nome, valor)) {
                std::cerr << "Falha ao escrever " << nome << std::endl;
                return false;
            }
        } catch (const std::exception &e) {
            std::cerr << "Erro ao escrever bias \"" << nome << "\": " << e.what() << std::endl;
            return false;
        }
        std::cout<< "Bias: "<< nome << "= "<< valor << " enviado para a câmera."<< std::endl;
    }

    // Obtém a interface de bias
    auto biases_b= cam.get_device().get_facility<Metavision::I_LL_Biases>();
    if (!biases_b) {
        std::cerr << "Interface I_LL_Biases não disponível." << std::endl;
        return false;
    }
    std::cout<<std::endl;
    std::cout<<"Valores atualizados lidos da câmera:"<<std::endl;
    std::cout<< "Valor lido do bias_diff_ON=" << biases_b->get("bias_diff_on")<<std::endl;
    std::cout<< "Valor lido do bias_diff_OFF=" << biases_b->get("bias_diff_off")<<std::endl;
    std::cout<< "Valor lido do bias_FO=" << biases_b->get("bias_fo")<<std::endl;
    std::cout<< "Valor lido do bias_HPF=" << biases_b->get("bias_hpf")<<std::endl;

    std::cout<< "Envio dos biases finalizado!!!"<< std::endl;

    return true;
}


*/

// Esta função pertence a classe "EventAnalyzer", conforme declarad m "funcoesEventos.hpp".
// Esta função será executada na chamada da callback da câmera de eventos semrep que um evento "CD" for disponibilizado:
// Ela será chamada sempre que novos eventos do tipo CD (Change Detection) estão disponíveis. 
// Para ser compatível com a callback, uma função precisa retornar void e ter dois parâmetros de entrqda. 
// Um para o "início" e outro para o "fim" do buffer de eventos que serão disponibilizados para a função. 
// Esses dois iteradores nos permitem acessar todos os novos eventos passados por meio dessa callback. 
// Essa função é equivalete a função definida abaixo (fora da classe) "count_events(const Metavision::EventCD *begin, const Metavision::EventCD *end)".
// É mais prático usar a função definida dentro da classe.
void EventAnalyzer::analyze_events(const Metavision::EventCD *begin, const Metavision::EventCD *end) {
    // time analysis
    // Note: events are ordered by timestamp in the callback, so the first event will have the lowest timestamp and
    // the last event will have the highest timestamp
    Metavision::timestamp min_t = begin->t;     // get the timestamp of the first event of this callback
    Metavision::timestamp max_t = (end - 1)->t; // get the timestamp of the last event of this callback
    global_max_t = max_t; // events are ordered by timestamp, so the current last event has the highest timestamp

    // O laço for permite acessar todos os eventos capturado nessa callback. 
    int counter = 0;
    for (const Metavision::EventCD *ev = begin; ev != end; ++ev) {
        ++counter; // Incrementa o contador de eventos
    }

    // Incrementa o contador global:
    global_counter += counter; 

    // Retirar o comentário das linnhas abaixo para exibir o relatório do buffer no terminal.
    // Atenção!!! Registrar informações no terminal pode reduzir drasticamente o desempenho da sua aplicação, 
    // especialmente em plataformas embarcadas com baixo poder computacional.
    // std::cout << "Cb n°" << callback_counter << ": " << counter << " events from t=" << min_t << " to t="
    // << max_t << " us." << std::endl;

    //Incrementa o contador de chamadas da callback:
    callback_counter++;
}



// Esta função será executada na chamada da callback da câmera de eventos semrep que um evento CD for disponibilizado:
// Ela será chamada sempre que novos eventos do tipo CD (Change Detection) estão disponíveis. 
// Para ser compatível com a callback, uma função precisa retornar void e ter dois parâmetros de entrqda. 
// Um para o "início" e outro para o "fim" do buffer de eventos que serão disponibilizados para a função. 
// Esses dois iteradores nos permitem acessar todos os novos eventos passados por meio dessa callback.  
void count_events(const Metavision::EventCD *begin, const Metavision::EventCD *end) {
    int counter = 0;

    // O laço for permite acessar todos os eventos capturado nessa callback. 
    for (const Metavision::EventCD *ev = begin; ev != end; ++ev) {
        ++counter; // Incrementa o contador de eventos

        // Exibe os valores do buffer de eventos: coordenadas, timestamp e a polaridade:
        std::cout << "Evento recebido: coordenadas (" << ev->x << ", " << ev->y << "), time-stamp: " << ev->t
                  << ", polaridade: " << ev->p << std::endl;
    }

    // report
    std::cout << "Ocorreram " << counter << " eventos nessa callback!!" << std::endl;
}


