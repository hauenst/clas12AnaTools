/* 
 * File:   DrawPedestals.cc
 * Author: rafopar
 *
 * Created on December 20, 2022, 8:59 PM
 */

#include <cstdlib>

using namespace std;

/*
 * 
 */
void DrawPedestals() {

    int run = 1295;
    const int n_ts = 6;

    TF1 *f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", 1000, 3500);
    f_Gaus->SetNpx(4500);

    TFile *file_in = new TFile(Form("CheckDecoding_%d.root", run), "Read");

    TH2D *h_ADC_chan = (TH2D*) file_in->Get("h_ADC_chan");
    TH2D * h_ADC_Chan_ts_[n_ts];
    TH2D *h_ADC_AllChan = (TH2D*)file_in->Get("h_ADC_AllChan");

    TGraph *gr_RMS = new TGraph();
    TGraph *gr_Mean = new TGraph();
    TGraph *gr_MeanFit = new TGraph();
    TGraph *gr_SigmFit = new TGraph();

    ofstream out_ped(Form("PedFiles/Peds_%d", run));

    int gr_index = 0;
    for (int ibin = 0; ibin < h_ADC_AllChan->GetNbinsX(); ibin++) {
        TH1D *h_tmp = (TH1D*) h_ADC_AllChan->ProjectionY(Form("h_ADC_ch_%d", ibin + 1), ibin + 1, ibin + 1);

        if (h_tmp->GetEntries() == 0) {
            continue;
        }

        int det_chan = int(h_ADC_AllChan->GetXaxis()->GetBinCenter(ibin + 1));

        double mean = h_tmp->GetMean();
        double rms = h_tmp->GetRMS();

        f_Gaus->SetParameters(h_tmp->GetMaximum(), mean, rms);
        h_tmp->Fit(f_Gaus, "MeV", "", mean - 4 * rms, mean + 4 * rms);
        double mean_fit = f_Gaus->GetParameter(1);
        double sigm_fit = f_Gaus->GetParameter(2);
        gr_RMS->SetPoint(gr_index, det_chan, rms);
        gr_Mean->SetPoint(gr_index, det_chan, mean);

        gr_MeanFit->SetPoint(gr_index, det_chan, mean_fit);
        gr_SigmFit->SetPoint(gr_index, det_chan, sigm_fit);

        out_ped << setw(5) << det_chan << setw(10) << mean << setw(10) << rms << endl;

        gr_index = gr_index + 1;
    }

    TCanvas *c1 = new TCanvas("c1", "", 1800, 600);
    c1->SetTopMargin(0.001);
    c1->SetLeftMargin(0.05);
    c1->SetRightMargin(0.005);

    gr_MeanFit->SetMarkerColor(2);
    gr_MeanFit->SetMarkerStyle(21);
    gr_SigmFit->SetMarkerColor(2);
    gr_SigmFit->SetMarkerStyle(21);

    c1->Clear();
    gr_Mean->SetMarkerStyle(20);
    gr_Mean->SetTitle("; Channel # ; RMS of ADC");
    gr_Mean->SetMarkerSize(0.5);
    gr_Mean->SetLineWidth(1);
    gr_Mean->SetMarkerColor(4);
    gr_Mean->GetYaxis()->SetTitleOffset(0.5);
    gr_Mean->Draw("APl");
    c1->Print(Form("Figs/ped_Mean_%d.pdf", run));
    c1->Print(Form("Figs/ped_Mean_%d.png", run));
    c1->Print(Form("Figs/ped_Mean_%d.root", run));

    gr_RMS->SetMarkerStyle(20);
    gr_RMS->SetTitle("; Channel # ; RMS of ADC");
    gr_RMS->SetMarkerSize(0.5);
    gr_RMS->SetLineWidth(1);
    gr_RMS->SetMarkerColor(4);
    gr_RMS->GetYaxis()->SetTitleOffset(0.5);
    gr_RMS->Draw("APl");
    c1->Print(Form("Figs/ped_RMS_globalVew_%d.pdf", run));
    c1->Print(Form("Figs/ped_RMS_globalVew_%d.png", run));
    c1->Print(Form("Figs/ped_RMS_globalVew_%d.root", run));


    gr_RMS->SetMaximum(14);
    gr_RMS->SetMinimum(0);
    c1->Modified();
    c1->Update();
    c1->SetGridy();
    c1->Print(Form("Figs/ped_RMS_ZoomSmallValues_%d.pdf", run));
    c1->Print(Form("Figs/ped_RMS_ZoomSmallValues_%d.png", run));
    c1->Print(Form("Figs/ped_RMS_ZoomSmallValues_%d.root", run));

    h_ADC_AllChan->SetStats(0);
    h_ADC_AllChan->SetTitle("; Channel # ; ADC");
    h_ADC_AllChan->SetTitleOffset(0.7, "Y");
    h_ADC_AllChan->Draw("colz");
    c1->Print(Form("Figs/ADC_vs_Channel_globalView_%d.pdf", run));
    c1->Print(Form("Figs/ADC_vs_Channel_globalView_%d.png", run));
    c1->Print(Form("Figs/ADC_vs_Channel_globalView_%d.root", run));

    h_ADC_chan->SetAxisRange(1160, 1220, "X");
    c1->Modified();
    c1->Update();
    c1->Print(Form("Figs/ADC_vs_Channel_Zoom_onXaxis_%d.pdf", run));
    c1->Print(Form("Figs/ADC_vs_Channel_Zoom_onXaxis_%d.png", run));
    c1->Print(Form("Figs/ADC_vs_Channel_Zoom_onXaxis_%d.proot", run));

    c1->Clear();
    TMultiGraph *mtgr_mean = new TMultiGraph();
    mtgr_mean->Add(gr_Mean);
    mtgr_mean->Add(gr_MeanFit);
    mtgr_mean->Draw("AP");

    c1->Clear();
    TMultiGraph *mtgr_sigm = new TMultiGraph();
    mtgr_sigm->Add(gr_RMS);
    mtgr_sigm->Add(gr_SigmFit);
    mtgr_sigm->Draw("AP");



    return 0;
}
