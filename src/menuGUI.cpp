
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QGroupBox>

#include <metavision/sdk/stream/camera.h>
#include <metavision/hal/facilities/i_ll_biases.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>

#include <iostream>
#include <thread>
#include <map>
#include <string>
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>

#include "menuGUI.hpp"
#include "eventoViewer.hpp" 

// Gera a interface GUI para interação com o usuário:
void launchMenuGUI(int argc, char *argv[]) {
    int argc_Qt = 0;
    char **argv_Qt = nullptr;
    QApplication app(argc_Qt, argv_Qt);

    int sizeWindow_H= 400;
    int sizeWindow_V= 800;
    
    int sizeHeigth_Label= 25;
    int sizeWidth_Label= 200;
    
    int sizeHeigth_Edt= 25;
    int sizeWidth_Edt= 60;

    int sizeHeigth_Bt= 25; 
    int sizeWidth_Bt= 160; 

    int posIniH_Bt= 20;
    int posIniV_Bt= 700;

    // Janela principal
    QWidget window;
    window.setWindowTitle("Menu Principal");
    window.resize(sizeWindow_H, sizeWindow_V);


    // **********************Botões de controle de eventos:*********************   
    // 
    QPushButton *btnVisualizar = new QPushButton("Visualizar eventos", &window);
    QPushButton *btnSair = new QPushButton("Sair", &window);
    QPushButton *btnRdBias = new QPushButton("Ler Biases", &window);
    QPushButton *btnSaveBias = new QPushButton("Salvar Biases (.json)", &window);
    QPushButton *btnReconfigFrameVirtual = new QPushButton("Config. frame virtual", &window);

    // Layout principal vertical
    QVBoxLayout *btnLayout = new QVBoxLayout();

    // Adiciona botões ao layout
    btnLayout->addWidget(btnVisualizar);
    btnLayout->addWidget(btnSair);
    btnLayout->addWidget(btnRdBias);
    btnLayout->addWidget(btnSaveBias);
    btnLayout->addWidget(btnReconfigFrameVirtual);

    // Posiciona os botões:
    btnRdBias->move(posIniH_Bt, posIniV_Bt-50);
    btnSaveBias->move(posIniH_Bt + 200, posIniV_Bt-50);
    btnReconfigFrameVirtual->move(posIniH_Bt, posIniV_Bt);
    btnVisualizar->move(posIniH_Bt + 200, posIniV_Bt);
    btnSair->move(posIniH_Bt, posIniV_Bt+50);

    // Dimensões dos botões:
    btnSair->setFixedSize(sizeWidth_Bt, sizeHeigth_Bt); 
    btnVisualizar->setFixedSize(sizeWidth_Bt, sizeHeigth_Bt); 
    btnRdBias->setFixedSize(sizeWidth_Bt, sizeHeigth_Bt); 
    btnSaveBias->setFixedSize(sizeWidth_Bt, sizeHeigth_Bt); 
    btnReconfigFrameVirtual->setFixedSize(sizeWidth_Bt, sizeHeigth_Bt); 

    // Cores dos botões:
    btnSair->setStyleSheet("color: red;");
    btnVisualizar->setStyleSheet("color: green;");


    // ******** Campos para "Tempo de acumulação (us)" **************************
    // Cria a subjanela para isolar os parãmetros da visualização:
    QGroupBox *subJanela1 = new QGroupBox("Configuração do frame:", &window);
    subJanela1->setAlignment(Qt::AlignLeft);
    subJanela1->setGeometry(10, 500, 380, 110); // posição (x, y), largura, altura
    subJanela1->setStyleSheet("QGroupBox::title { color: green; }");

    QLabel *labelAcc = new QLabel("Duração do frame [µs]:", subJanela1); 
    labelAcc->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelAcc->move(80, 35);
    labelAcc->setAlignment(Qt::AlignRight | Qt::AlignVCenter);   
         
    QLineEdit *editAcc = new QLineEdit(subJanela1);
    editAcc->setText(QString::number(20000));  // valor inicial
    editAcc->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editAcc->move(300, 35); 
    editAcc->setAlignment(Qt::AlignRight);


    // *********** Campos para "Frame por segundo (Hz)"****************************
    QLabel *labelFps = new QLabel("Frames por segundo [Hz]:", subJanela1);
    labelFps->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelFps->move(80, 70);
    labelFps->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editFps = new QLineEdit(subJanela1);
    //Inicializa o campo com o valor de 50:
    editFps->setText(QString::number(50)); 
    editFps->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editFps->move(300,70); 
    editFps->setAlignment(Qt::AlignRight);


    // *********** Campos para exibir o BiasDiff lidos da camera **************
    // Subjanela para isolar os parãmetros dos valores dos biases lidos da câmera:
    QGroupBox *subJanela2 = new QGroupBox("Biases lidos da câmera:", &window);
    subJanela2->setAlignment(Qt::AlignLeft);
    subJanela2->setGeometry(10, 10, 380, 250); // posição (x, y), largura, altura
    subJanela2->setStyleSheet("QGroupBox::title { color: green; }");

    QLabel *labelBiasDiff = new QLabel("Bias: diff:", subJanela2);
    labelBiasDiff->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBiasDiff->move(80, 35);
    labelBiasDiff->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBiasDiffRead = new QLineEdit(subJanela2);
    //Inicializa o campo com o valor de 50:
    editBiasDiffRead->setText("--");;  // valor inicial
    editBiasDiffRead->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBiasDiffRead->move(300, 35); 
    editBiasDiffRead->setAlignment(Qt::AlignRight);
    editBiasDiffRead->setReadOnly(true);
    editBiasDiffRead->setStyleSheet("background-color: #f0f0f0;");

    QLabel *labelBiasDiff_ON = new QLabel("Bias: diff_ON:", subJanela2);
    labelBiasDiff_ON->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBiasDiff_ON->move(80, 70);
    labelBiasDiff_ON->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBiasDiff_ONRead = new QLineEdit(subJanela2);
    //Inicializa o campo com o valor de 50:
    editBiasDiff_ONRead->setText("--");;  // valor inicial
    editBiasDiff_ONRead->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBiasDiff_ONRead->move(300,70); 
    editBiasDiff_ONRead->setAlignment(Qt::AlignRight);    
    editBiasDiff_ONRead->setReadOnly(true);
    editBiasDiff_ONRead->setStyleSheet("background-color: #f0f0f0;");

    QLabel *labelBiasDiff_OFF = new QLabel("Bias: diff_OFF:", subJanela2);
    labelBiasDiff_OFF->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBiasDiff_OFF->move(80, 105);
    labelBiasDiff_OFF->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBiasDiff_OFFRead = new QLineEdit(subJanela2);
    //Inicializa o campo com o valor de 50:
    editBiasDiff_OFFRead->setText("--");;  // valor inicial
    editBiasDiff_OFFRead->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBiasDiff_OFFRead->move(300,105); 
    editBiasDiff_OFFRead->setAlignment(Qt::AlignRight);
    editBiasDiff_OFFRead->setReadOnly(true);
    editBiasDiff_OFFRead->setStyleSheet("background-color: #f0f0f0;");

    QLabel *labelBias_FO = new QLabel("Bias: FO:", subJanela2);
    labelBias_FO->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBias_FO->move(80,140);
    labelBias_FO->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBias_FORead = new QLineEdit(subJanela2);
    //Inicializa o campo com o valor de 50:
    editBias_FORead->setText("--");  // valor inicial
    editBias_FORead->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBias_FORead->move(300,140); 
    editBias_FORead->setAlignment(Qt::AlignRight);
    editBias_FORead->setReadOnly(true);
    editBias_FORead->setStyleSheet("background-color: #f0f0f0;");

    QLabel *labelBias_HPF = new QLabel("Bias: HPF:", subJanela2);
    labelBias_HPF->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBias_HPF->move(80,175);
    labelBias_HPF->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBias_HPFRead = new QLineEdit(subJanela2);
    //Inicializa o campo com o valor de 50:
    editBias_HPFRead->setText("--");;  // valor inicial
    editBias_HPFRead->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBias_HPFRead->move(300,175); 
    editBias_HPFRead->setAlignment(Qt::AlignRight);
    editBias_HPFRead->setReadOnly(true);
    editBias_HPFRead->setStyleSheet("background-color: #f0f0f0;");

    QLabel *labelBias_REFR = new QLabel("Bias: REFR:",subJanela2);
    labelBias_REFR->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBias_REFR->move(80,210);
    labelBias_REFR->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBias_REFRRead = new QLineEdit(subJanela2);
    //Inicializa o campo com o valor de 50:
    editBias_REFRRead->setText("--");  // valor inicial
    editBias_REFRRead->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBias_REFRRead->move(300,210); 
    editBias_REFRRead->setAlignment(Qt::AlignRight);
    editBias_REFRRead->setReadOnly(true);
    editBias_REFRRead->setStyleSheet("background-color: #f0f0f0;");



    // *********** Campos para editar o BiasDiff para escrita na camera **************
    // Subjanela para isolar os parãmetros dos valores dos biases lidos da câmera:
    QGroupBox *subJanela3 = new QGroupBox("Biases para configurar a câmera:", &window);
    subJanela3->setAlignment(Qt::AlignLeft);
    subJanela3->setGeometry(10, 280, 380, 200); // posição (x, y), largura, altura
    subJanela3->setStyleSheet("QGroupBox::title { color: green; }");

    QLabel *labelBiasDiff_ON_WR = new QLabel("Bias: diff_ON:", subJanela3);
    labelBiasDiff_ON_WR->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBiasDiff_ON_WR->move(80,35);
    labelBiasDiff_ON_WR->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBiasDiff_ON_WR = new QLineEdit(subJanela3);
    //Inicializa o campo com o valor de 50:
    editBiasDiff_ON_WR->setText("--");;  // valor inicial
    editBiasDiff_ON_WR->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBiasDiff_ON_WR->move(300,35); 
    editBiasDiff_ON_WR->setAlignment(Qt::AlignRight);    

     QLabel *labelBiasDiff_OFF_WR = new QLabel("Bias: diff_OFF:", subJanela3);
    labelBiasDiff_OFF_WR->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBiasDiff_OFF_WR->move(80,70);
    labelBiasDiff_OFF_WR->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBiasDiff_OFF_WR = new QLineEdit(subJanela3);
    //Inicializa o campo com o valor de 50:
    editBiasDiff_OFF_WR->setText("--");;  // valor inicial
    editBiasDiff_OFF_WR->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBiasDiff_OFF_WR->move(300,70); 
    editBiasDiff_OFF_WR->setAlignment(Qt::AlignRight);    

    QLabel *labelBias_FO_WR = new QLabel("Bias: FO:", subJanela3);
    labelBias_FO_WR->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBias_FO_WR->move(80,105);
    labelBias_FO_WR->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBias_FO_WR = new QLineEdit(subJanela3);
    //Inicializa o campo com o valor de 50:
    editBias_FO_WR->setText("--");  // valor inicial
    editBias_FO_WR->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBias_FO_WR->move(300,105); 
    editBias_FO_WR->setAlignment(Qt::AlignRight);   

    QLabel *labelBias_HPF_WR = new QLabel("Bias: HPF:", subJanela3);
    labelBias_HPF_WR->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBias_HPF_WR->move(80,140);
    labelBias_HPF_WR->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBias_HPF_WR = new QLineEdit(subJanela3);
    //Inicializa o campo com o valor de 50:
    editBias_HPF_WR->setText("--");;  // valor inicial
    editBias_HPF_WR->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBias_HPF_WR->move(300,140); 
    editBias_HPF_WR->setAlignment(Qt::AlignRight);

    QLabel *labelBias_REFR_WR = new QLabel("Bias: REFR:",subJanela3);
    labelBias_REFR_WR->setFixedSize(sizeWidth_Label, sizeHeigth_Label);
    labelBias_REFR_WR->move(80,210);
    labelBias_REFR_WR->setAlignment(Qt::AlignRight | Qt::AlignVCenter); 
   
    QLineEdit *editBias_REFR_WR = new QLineEdit(subJanela3);
    //Inicializa o campo com o valor de 50:
    editBias_REFR_WR->setText("--");  // valor inicial
    editBias_REFR_WR->setFixedSize(sizeWidth_Edt, sizeHeigth_Edt);
    editBias_REFR_WR->move(300,210); 
    editBias_REFR_WR->setAlignment(Qt::AlignRight);



    // ************* Conecta botão "Visualizar eventos" **************************
    QObject::connect(btnVisualizar, &QPushButton::clicked, [&]() {
        
        //std::thread viewer_thread([]() {
            frameEventView(argc, argv); // Essa função NÃO deve usar widgets Qt
        //});
        //viewer_thread.detach(); // Executa paralelamente sem bloquear a GUI   
        std::cout<<"Passei aqui!"<<std::endl; 

        /*
        // Extrai os valores numéicos dos campos acc e fps:
        QString accStr = editAcc->text();
        QString fpsStr = editFps->text();

        // Define duas variáveis booleanas para testar se a convresão de string para interio foi ok:
        bool ok1 = false, ok2 = false;

        // COnverte de string para inteiro, se der certo, ok1 e ok2 receberão "true":
        int acc_us = accStr.toInt(&ok1);
        int fps_hz = fpsStr.toInt(&ok2);

        // Testa a conversão de variável string para inteiro:
        if (!ok1 || !ok2 || acc_us <= 0 || fps_hz <= 0) {
            std::cerr << "Valores inválidos de tempo ou FPS!" << std::endl;
            return;
        }

        std::cout << "Botão 'Visualizar eventos' pressionado." << std::endl;
        std::cout << "acc = " << acc_us << " µs, fps = " << fps_hz << " Hz" << std::endl;

        // Aqui você pode passar os parâmetros acc_us e fps_hz para a função frameEventView,
        // modificando a assinatura dela para aceitar esses argumentos, se necessário.
        frameEventView(); // adaptar depois */
    });


    
    // ************* Conecta botão "Ler Biases" **************************
    QObject::connect(btnRdBias, &QPushButton::clicked, [&]() {
            // Variavel struct de ponteiros para os campos de biases (apenas leitura) da interface GUI:
        BiasFields_RD campos_bias_RD = {
            editBiasDiffRead,
            editBiasDiff_ONRead,
            editBiasDiff_OFFRead,
            editBias_FORead,
            editBias_HPFRead,
            editBias_REFRRead
        }; 
        
        // Variavel struct de ponteiros para os campos de escrita dos Biases da interface GUI:   
        BiasFields_WR campos_bias_WR = {
            editBiasDiff_ON_WR,
            editBiasDiff_OFF_WR,
            editBias_FO_WR,
            editBias_HPF_WR
        }; 
        ler_biases(campos_bias_RD, campos_bias_WR);

    });


    // ************** Conecta botão para escrita dos Biases na cãmera **********************
    QObject::connect(btnSaveBias, &QPushButton::clicked, [&]() {        
    BiasFields_RD campos_bias_RD = {
        editBiasDiffRead,
        editBiasDiff_ONRead,
        editBiasDiff_OFFRead,
        editBias_FORead,
        editBias_HPFRead,
        editBias_REFRRead
    }; 
    // Variavel struct de ponteiros para os campos de escrita dos Biases da interface GUI:   
    BiasFields_WR campos_bias_WR = {
        editBiasDiff_ON_WR,
        editBiasDiff_OFF_WR,
        editBias_FO_WR,
        editBias_HPF_WR
    }; 

    // Chama a função que escreve os valores dos biases em arquivo .json
    salvar_biases_em_json(campos_bias_RD, campos_bias_WR);
    });



    // ************** Conecta botão ao evento de saída do aplicativo**********************
    QObject::connect(btnSair, &QPushButton::clicked, [&]() {
    std::cout << "Botão 'Sair' pressionado." << std::endl << "Aplicação finalizada..." << std::endl;
    app.quit();
    
    });


    // Variavel struct de ponteiros para os campos de biases (apenas leitura) da interface GUI:
    BiasFields_RD campos_bias_RD = {
        editBiasDiffRead,
        editBiasDiff_ONRead,
        editBiasDiff_OFFRead,
        editBias_FORead,
        editBias_HPFRead,
        editBias_REFRRead
    }; 
    
    // Variavel struct de ponteiros para os campos de escrita dos Biases da interface GUI:   
    BiasFields_WR campos_bias_WR = {
        editBiasDiff_ON_WR,
        editBiasDiff_OFF_WR,
        editBias_FO_WR,
        editBias_HPF_WR
    }; 

    // Chamada da função qua atualzia os campos de bias de leitura e escrita da interface GUI:
    ler_biases(campos_bias_RD, campos_bias_WR);

    // Exibe o fromulário GUI e executa na forma de loop:    
    window.show();
    app.exec();
}



// Função chamada para ler os biases da câmera:
void ler_biases(BiasFields_RD &camposRD, BiasFields_WR &camposWR) {
    // Só para limpar o terminal:
    limpar_terminal();

    // Instancia um objeto cam com escopo global:
    Metavision::Camera cam;

   // Cria um objeto "config", definido no SDK Metavision, que será um "container" para os parâmetros de configuração da câmera:
    Metavision::DeviceConfig config;

    // Atenção!!!!
    // Para setar os valores dos biases, para aversão SDK 4.6 é necessaŕio usa a funçao "config.enable_biases_range_check_bypass(true)".
    // Cuidado, não usar valores fora da faixa permitida para os sensor. Verificar os valores em:
    // https://docs.prophesee.ai/stable/hw/manuals/biases.html    
    config.enable_biases_range_check_bypass(true);
    
    // Abre a primeira cãmera disponível na USB, e já passa a configuração através da variável "config":
    try {
        cam = Metavision::Camera::from_first_available(config);
    } 
    catch (const std::exception &e) {
        std::cerr << "Erro ao abrir a câmera para carregar os biases: " << e.what() << std::endl;
        return;
    }

    // Efetua a leitura dos biases na câmera de eventos:
    auto biases = cam.get_device().get_facility<Metavision::I_LL_Biases>();
    // Testa sea aleitura foi ok:
    if (!biases) {
        std::cerr << "Interface I_LL_Biases não disponível." << std::endl;
        return;
    }

    std::map<std::string, int> bias_map = biases->get_all_biases();
            
    if (bias_map.count("bias_diff"))
        camposRD.diff->setText(QString::number(bias_map.at("bias_diff")));

    if (bias_map.count("bias_diff_on")){
        camposRD.diff_on->setText(QString::number(bias_map.at("bias_diff_on")));
        camposWR.diff_on->setText(QString::number(bias_map.at("bias_diff_on")));
    }

    if (bias_map.count("bias_diff_off")){
        camposRD.diff_off->setText(QString::number(bias_map.at("bias_diff_off")));
        camposWR.diff_off->setText(QString::number(bias_map.at("bias_diff_off")));
    }

    if (bias_map.count("bias_fo")){
        camposRD.fo->setText(QString::number(bias_map.at("bias_fo")));
        camposWR.fo->setText(QString::number(bias_map.at("bias_fo")));        
    }

    if (bias_map.count("bias_hpf")){
        camposRD.hpf->setText(QString::number(bias_map.at("bias_hpf")));        
        camposWR.hpf->setText(QString::number(bias_map.at("bias_hpf"))); 
    }
    
    if (bias_map.count("bias_refr"))
        camposRD.refr->setText(QString::number(bias_map.at("bias_refr")));
}


// FUnção usada para escrever os valors dos biases na câmera:
bool escrever_biases_cam(const BiasFields_WR &campos_bias) {
    // Só para limpar o terminal:
    limpar_terminal();

    // Instancia um objeto cam com escopo global:
    Metavision::Camera cam;

   // Cria um objeto "config", definido no SDK Metavision, que será um "container" para os parâmetros de configuração da câmera:
    Metavision::DeviceConfig config;

    // Atenção!!!!
    // Para setar os valores dos biases, para aversão SDK 4.6 é necessaŕio usa a funçao "config.enable_biases_range_check_bypass(true)".
    // Cuidado, não usar valores fora da faixa permitida para os sensor. Verificar os valores em:
    // https://docs.prophesee.ai/stable/hw/manuals/biases.html    
    config.enable_biases_range_check_bypass(true);
    
    // Abre a primeira cãmera disponível na USB, e já passa a configuração através da variável "config":
    try {
        cam = Metavision::Camera::from_first_available(config);
    } 
    catch (const std::exception &e) {
        std::cerr << "Erro ao abrir a câmera para carregar os biases: " << e.what() << std::endl;
        return false;
    }   

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


void salvar_biases_em_json(BiasFields_RD &camposRD, BiasFields_WR &camposWR) {
    const std::string &nome_arquivo= "../biases.json";

    // Cria objeto JSON diretamente do bias_map:
    nlohmann::ordered_json j;
    j["bias_diff"]= camposRD.diff->text().toInt();  // Recebe o valor do RD;       
    j["bias_diff_on"]= camposRD.diff_on->text().toInt();  // Recebe o valor do RD;       
    j["bias_diff_off"]= camposRD.diff_off->text().toInt(); // Recebe o valor do WR;  
    j["bias_fo"]= camposWR.fo->text().toInt(); // Recebe o valor do WR;  
    j["bias_hpf"]= camposWR.hpf->text().toInt(); // Recebe o valor do WR;  
    j["bias_refr"]= camposRD.refr->text().toInt();// Recebe o valor do RD;      

    // Salva arquivo .jason
    std::ofstream arquivo_json(nome_arquivo);
    if (!arquivo_json) {
        std::cerr << "Erro ao abrir arquivo para escrita: " << nome_arquivo << std::endl;
        return;
    }

    arquivo_json << j.dump(4);  // Indentação com 4 espaços
    std::cout << "Biases salvos com sucesso em: " << nome_arquivo << std::endl;
}
