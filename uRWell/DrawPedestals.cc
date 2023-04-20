/*
 * File:   DrawPedestals.cc
 * Author: rafopar
 *
 * Created on December 20, 2022, 8:59 PM
 */

#include <cstdlib>

using namespace std;

void DrawHybridLimits(TGraph*, double);

void CalculateMeanAndSigma(TH2D*,std::map< int, double >&, std::map< int, double >&, std::map< int, double >&, std::map< int, double >&);
void MakeTGraphs(TGraph*, TGraph*, TGraph*, TGraph*, std::map< int, double >&, std::map< int, double >&, std::map< int, double >&, std::map< int, double >&);
void WriteToFileAll(TString, std::map< int, double >&, std::map< int, double >&, std::map< int, double >&, std::map< int, double >& );
void WriteToFile(TString, std::map< int, double >&, std::map< int, double >&, std::map< int, double >&, std::map< int, double >& );
void TCanvasPrint(TCanvas*, TString);

/*
 *
 */
void DrawPedestals(int run) {

    if (run < 0) {cout << "Run number has to be above 0. End program " << endl; exit;}
    const int n_ts = 15;
    const int n_urwellapvs = 12;

    TFile *file_in = new TFile(Form("CheckDecoding_%d_0.root", run), "Read");

    TFile *file_out = new TFile(Form("Plots_DrawPedestals_%d_0.root", run), "RECREATE");


    //urwell ADC histograms for unique channel
    TH2D *h2_ADC_uniquechan = (TH2D*) file_in->Get("h2_ADC_uniquechan");
    TH2D *h2_ADC_uniquechan_ts[n_ts];
    TH2D *h2_ADCaver_uniquechan = (TH2D*)file_in->Get("h2_ADCaver_uniquechan");

    TGraph *gr_RMS = new TGraph();
    TGraph *gr_Mean = new TGraph();
    TGraph *gr_MeanFit = new TGraph();
    TGraph *gr_SigmaFit = new TGraph();
    gr_MeanFit->SetMarkerColor(2);
    gr_MeanFit->SetMarkerStyle(21);
    gr_SigmaFit->SetMarkerColor(2);
    gr_SigmaFit->SetMarkerStyle(21);


    //Map Values to take care of Mean, RMS and Sigma values, keys are channel numbers
    std::map< int, double > mean;
    std::map< int, double > rms;
    std::map< int, double > mean_fit;
    std::map< int, double > sigma_fit;


    //Single Canvas to save single plots as PDF
    TCanvas *canv_single = new TCanvas("canv_single", "", 1800, 600);

    //Determine Values for urwell with all channels combined
    CalculateMeanAndSigma(h2_ADCaver_uniquechan, mean, rms, mean_fit, sigma_fit );
    MakeTGraphs(gr_Mean, gr_RMS, gr_MeanFit, gr_SigmaFit, mean, rms, mean_fit, sigma_fit);
    WriteToFile(Form("PedFiles/PedestalUrwell_uniqueID_%d", run), mean, rms, mean_fit, sigma_fit);
    WriteToFileAll(Form("PedFiles/AllMapInfo_%d", run), mean, rms, mean_fit, sigma_fit);

    TCanvas *c1 = new TCanvas("c1", "", 1800, 600);
    c1->SetTopMargin(0.001);
    c1->SetLeftMargin(0.05);
    c1->SetRightMargin(0.005);

    c1->Clear();
    c1->Divide(2,2);
    c1->cd(1);
    gr_Mean->SetMarkerStyle(20);
    gr_Mean->SetTitle("; Channel # ; Mean of ADC");
    gr_Mean->SetMarkerSize(0.5);
    gr_Mean->SetLineWidth(1);
    gr_Mean->SetMarkerColor(4);
    gr_Mean->GetYaxis()->SetTitleOffset(0.5);
    gr_Mean->Draw("APl");
    DrawHybridLimits(gr_Mean, 120);
    c1->cd(2);
    gr_RMS->SetMarkerStyle(20);
    gr_RMS->SetTitle("; Channel # ; RMS of ADC");
    gr_RMS->SetMarkerSize(0.5);
    gr_RMS->SetLineWidth(1);
    gr_RMS->SetMarkerColor(4);
    gr_RMS->GetYaxis()->SetTitleOffset(0.5);
    gr_RMS->Draw("APl");
    DrawHybridLimits(gr_RMS, 25);
    c1->cd(3);
    gr_MeanFit->SetMarkerStyle(20);
    gr_MeanFit->SetTitle("; Channel # ; Mean of ADC");
    gr_MeanFit->SetMarkerSize(0.5);
    gr_MeanFit->SetLineWidth(1);
    gr_MeanFit->SetMarkerColor(4);
    gr_MeanFit->GetYaxis()->SetTitleOffset(0.5);
    gr_MeanFit->Draw("APl");
    DrawHybridLimits(gr_Mean, 120);
    c1->cd(4);
    gr_SigmaFit->SetMarkerStyle(20);
    gr_SigmaFit->SetTitle("; Channel # ; Sigma of ADC");
    gr_SigmaFit->SetMarkerSize(0.5);
    gr_SigmaFit->SetLineWidth(1);
    gr_SigmaFit->SetMarkerColor(4);
    gr_SigmaFit->GetYaxis()->SetTitleOffset(0.5);
    gr_SigmaFit->Draw("APl");
    DrawHybridLimits(gr_SigmaFit, 25);
    TCanvasPrint(c1, TString("Figs/meansigma_vs_uniquechannel_"+to_string(run) ));

    file_out->cd();
    gr_Mean->Write("Mean_all_channels");
    gr_RMS->Write("RMS_all_channels");

/*
    c1->Clear();
    TMultiGraph *mtgr_mean = new TMultiGraph();
    mtgr_mean->Add(gr_Mean);
    mtgr_mean->Add(gr_MeanFit);
    mtgr_mean->Draw("AP");

    c1->Clear();
    TMultiGraph *mtgr_sigm = new TMultiGraph();
    mtgr_sigm->Add(gr_RMS);
    mtgr_sigm->Add(gr_SigmaFit);
    mtgr_sigm->Draw("AP");
*/

    //urwell: Loop to determine pedestals/means/sigmas for each APV slot and save as figures and root files
    TCanvas *c4 = new TCanvas("c4", "", 1800, 600);
    c4->Divide(3,4);
    TCanvas *c5 = new TCanvas("c5", "", 1800, 600);
    c5->Divide(3,4);
    TGraph *gr_RMS_apvchan[n_urwellapvs];
    TGraph *gr_Mean_apvchan[n_urwellapvs];
    TGraph *gr_MeanFit_apvchan[n_urwellapvs];
    TGraph *gr_SigmaFit_apvchan[n_urwellapvs];
    TGraph *gr_RMS_stripchan[n_urwellapvs];
    TGraph *gr_Mean_stripchan[n_urwellapvs];
    TGraph *gr_MeanFit_stripchan[n_urwellapvs];
    TGraph *gr_SigmaFit_stripchan[n_urwellapvs];
    //Get all histograms and initialize TGraphs;
    TH2D *h2_ADCaver_apvchan_apvslot[n_urwellapvs];
    TH2D *h2_ADCaver_stripchan_apvslot[n_urwellapvs];
    for (int j = 0; j < n_urwellapvs; j++) {
        h2_ADCaver_apvchan_apvslot[j] = (TH2D*) file_in->Get(Form("h2_ADCaver_apvchan_apvslot_%d", j));
        c4->cd(j+1);
        h2_ADCaver_apvchan_apvslot[j]->Draw("COLZ");
        c5->cd(j+1);
        h2_ADCaver_stripchan_apvslot[j]= (TH2D*) file_in->Get(Form("h2_ADCaver_stripchan_apvslot_%d", j));
        h2_ADCaver_stripchan_apvslot[j]->Draw("COLZ");
        gr_RMS_apvchan[j] = new TGraph();
        gr_Mean_apvchan[j] = new TGraph();
        gr_MeanFit_apvchan[j] = new TGraph();
        gr_SigmaFit_apvchan[j] = new TGraph();
        gr_RMS_stripchan[j] = new TGraph();
        gr_Mean_stripchan[j] = new TGraph();
        gr_MeanFit_stripchan[j] = new TGraph();
        gr_SigmaFit_stripchan[j] = new TGraph();
    }
    TCanvasPrint(c4, TString("Figs/Overview_ADCaver_apvchan_apvslot_"+to_string(run) ));
    TCanvasPrint(c5, TString("Figs/Overview_ADCaver_stripchan_apvslot_"+to_string(run) ));

    TCanvas *c2 = new TCanvas("c2", "", 1800, 600);
    c2->Divide(2,2);
    TCanvas *c3 = new TCanvas("c3", "", 1800, 600);
    c3->Divide(2,2);

    for (int apv = 0; apv < n_urwellapvs; apv++) {
      mean.clear();
      rms.clear();
      mean_fit.clear();
      sigma_fit.clear();
      CalculateMeanAndSigma(h2_ADCaver_apvchan_apvslot[apv], mean, rms, mean_fit, sigma_fit );
      MakeTGraphs(gr_Mean_apvchan[apv], gr_RMS_apvchan[apv], gr_MeanFit_apvchan[apv], gr_SigmaFit_apvchan[apv], mean, rms, mean_fit, sigma_fit);
      c2->Clear();
      c2->Divide(2,2);
      c2->cd(1);
      gr_Mean_apvchan[apv]->SetMarkerStyle(20);
      gr_Mean_apvchan[apv]->SetTitle("; APV Channel # ; Mean of ADC");
      gr_Mean_apvchan[apv]->SetMarkerSize(0.5);
      gr_Mean_apvchan[apv]->SetLineWidth(1);
      gr_Mean_apvchan[apv]->SetMarkerColor(4);
      gr_Mean_apvchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_Mean_apvchan[apv]->Draw("APl");
      c2->cd(2);
      gr_RMS_apvchan[apv]->SetMarkerStyle(20);
      gr_RMS_apvchan[apv]->SetTitle("; APV Channel # ; RMS of ADC");
      gr_RMS_apvchan[apv]->SetMarkerSize(0.5);
      gr_RMS_apvchan[apv]->SetLineWidth(1);
      gr_RMS_apvchan[apv]->SetMarkerColor(4);
      gr_RMS_apvchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_RMS_apvchan[apv]->Draw("APl");
      c2->cd(3);
      gr_MeanFit_apvchan[apv]->SetMarkerStyle(20);
      gr_MeanFit_apvchan[apv]->SetTitle("; APV Channel # ; Mean of ADC");
      gr_MeanFit_apvchan[apv]->SetMarkerSize(0.5);
      gr_MeanFit_apvchan[apv]->SetLineWidth(1);
      gr_MeanFit_apvchan[apv]->SetMarkerColor(4);
      gr_MeanFit_apvchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_MeanFit_apvchan[apv]->Draw("APl");
      c2->cd(4);
      gr_SigmaFit_apvchan[apv]->SetMarkerStyle(20);
      gr_SigmaFit_apvchan[apv]->SetTitle("; APV Channel # ; Sigma of ADC");
      gr_SigmaFit_apvchan[apv]->SetMarkerSize(0.5);
      gr_SigmaFit_apvchan[apv]->SetLineWidth(1);
      gr_SigmaFit_apvchan[apv]->SetMarkerColor(4);
      gr_SigmaFit_apvchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_SigmaFit_apvchan[apv]->Draw("APl");
      TCanvasPrint(c2, TString("Figs/meansigma_apvchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
    //  c2->Print(Form("Figs/meansigma_apvchan_apv_%d_run_%d.pdf", apv, run));
    //  c2->Print(Form("Figs/meansigma_apvchan_apv_%d_run_%d.png", apv, run));
    //  c2->Print(Form("Figs/meansigma_apvchan_apv_%d_run_%d.root", apv, run));

      mean.clear();
      rms.clear();
      mean_fit.clear();
      sigma_fit.clear();
      CalculateMeanAndSigma(h2_ADCaver_stripchan_apvslot[apv], mean, rms, mean_fit, sigma_fit );
      MakeTGraphs(gr_Mean_stripchan[apv], gr_RMS_stripchan[apv], gr_MeanFit_stripchan[apv], gr_SigmaFit_stripchan[apv], mean, rms, mean_fit, sigma_fit);
      WriteToFile(Form("PedFiles/PedestalUrwell_apv%i_stripchannel__%d", apv, run), mean, rms, mean_fit, sigma_fit);
      c3->Clear();
      c3->Divide(2,2);
      c3->cd(1);
      gr_Mean_stripchan[apv]->SetMarkerStyle(20);
      gr_Mean_stripchan[apv]->SetTitle("; Strip Channel # ; Mean of ADC");
      gr_Mean_stripchan[apv]->SetMarkerSize(0.5);
      gr_Mean_stripchan[apv]->SetLineWidth(1);
      gr_Mean_stripchan[apv]->SetMarkerColor(4);
      gr_Mean_stripchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_Mean_stripchan[apv]->Draw("APl");
      c3->cd(2);
      gr_RMS_stripchan[apv]->SetMarkerStyle(20);
      gr_RMS_stripchan[apv]->SetTitle("; Strip Channel # ; RMS of ADC");
      gr_RMS_stripchan[apv]->SetMarkerSize(0.5);
      gr_RMS_stripchan[apv]->SetLineWidth(1);
      gr_RMS_stripchan[apv]->SetMarkerColor(4);
      gr_RMS_stripchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_RMS_stripchan[apv]->Draw("APl");
      c3->cd(3);
      gr_MeanFit_stripchan[apv]->SetMarkerStyle(20);
      gr_MeanFit_stripchan[apv]->SetTitle("; Strip Channel # ; Mean of ADC");
      gr_MeanFit_stripchan[apv]->SetMarkerSize(0.5);
      gr_MeanFit_stripchan[apv]->SetLineWidth(1);
      gr_MeanFit_stripchan[apv]->SetMarkerColor(4);
      gr_MeanFit_stripchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_MeanFit_stripchan[apv]->Draw("APl");
      c3->cd(4);
      gr_SigmaFit_stripchan[apv]->SetMarkerStyle(20);
      gr_SigmaFit_stripchan[apv]->SetTitle("; Strip Channel # ; Sigma of ADC");
      gr_SigmaFit_stripchan[apv]->SetMarkerSize(0.5);
      gr_SigmaFit_stripchan[apv]->SetLineWidth(1);
      gr_SigmaFit_stripchan[apv]->SetMarkerColor(4);
      gr_SigmaFit_stripchan[apv]->GetYaxis()->SetTitleOffset(0.5);
      gr_SigmaFit_stripchan[apv]->Draw("APl");
      TCanvasPrint(c3, TString("Figs/meansigma_stripchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
    //  c3->Print(Form("Figs/meansigma_stripchan_apv_%d_run_%d.pdf", apv, run));
    //  c3->Print(Form("Figs/meansigma_stripchan_apv_%d_run_%d.png", apv, run));
    //  c3->Print(Form("Figs/meansigma_stripchan_apv_%d_run_%d.root", apv, run));
      canv_single->Clear();
      canv_single->cd();
      gr_Mean_stripchan[apv]->Draw("APl");
      TCanvasPrint(canv_single, TString("Figs/SingleAPVs/mean_stripchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
      canv_single->Clear();
      canv_single->cd();
      gr_RMS_stripchan[apv]->Draw("APl");
      TCanvasPrint(canv_single, TString("Figs/SingleAPVs/rms_stripchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
      canv_single->Clear();

      file_out->cd();
      gr_Mean_apvchan[apv]->Write(Form("Mean_apvchan_apv_%i",apv));
      gr_RMS_apvchan[apv]->Write(Form("RMS_apvchan_apv_%i",apv));
      gr_Mean_stripchan[apv]->Write(Form("Mean_stripchan_apv_%i",apv));
      gr_RMS_stripchan[apv]->Write(Form("RMS_stripchan_apv_%i",apv));
    }

  //GEM: Loop to determine pedestals/means/sigmas for each APV slot and save as figures and root files
  const int n_gemapvs = 2;
  TGraph *gr_RMS_apvchan_GEM[n_gemapvs];
  TGraph *gr_Mean_apvchan_GEM[n_gemapvs];
  TGraph *gr_MeanFit_apvchan_GEM[n_gemapvs];
  TGraph *gr_SigmaFit_apvchan_GEM[n_gemapvs];
  TGraph *gr_RMS_stripchan_GEM[n_gemapvs];
  TGraph *gr_Mean_stripchan_GEM[n_gemapvs];
  TGraph *gr_MeanFit_stripchan_GEM[n_gemapvs];
  TGraph *gr_SigmaFit_stripchan_GEM[n_gemapvs];
  //Get all histograms and initialize TGraphs;
  TH2D *h2_GEM_ADCaver_apvchan_apvslot[n_gemapvs];
  TH2D *h2_GEM_ADCaver_stripchan_apvslot[n_gemapvs];
  for (int j = 0; j < n_gemapvs; j++) {
      h2_GEM_ADCaver_apvchan_apvslot[j] = (TH2D*) file_in->Get(Form("h2_GEM_ADCaver_apvchan_apvslot_%d", j));
      h2_GEM_ADCaver_stripchan_apvslot[j]= (TH2D*) file_in->Get(Form("h2_GEM_ADCaver_stripchan_apvslot_%d", j));
      gr_RMS_apvchan_GEM[j] = new TGraph();
      gr_Mean_apvchan_GEM[j] = new TGraph();
      gr_MeanFit_apvchan_GEM[j] = new TGraph();
      gr_SigmaFit_apvchan_GEM[j] = new TGraph();
      gr_RMS_stripchan_GEM[j] = new TGraph();
      gr_Mean_stripchan_GEM[j] = new TGraph();
      gr_MeanFit_stripchan_GEM[j] = new TGraph();
      gr_SigmaFit_stripchan_GEM[j] = new TGraph();
  }

  TCanvas *c2_gem = new TCanvas("c2_gem", "", 1800, 600);
  c2_gem->Divide(2,2);
  TCanvas *c3_gem = new TCanvas("c3_gem", "", 1800, 600);
  c3_gem->Divide(2,2);

  for (int apv = 0; apv < n_gemapvs; apv++) {
    mean.clear();
    rms.clear();
    mean_fit.clear();
    sigma_fit.clear();
    CalculateMeanAndSigma(h2_GEM_ADCaver_apvchan_apvslot[apv], mean, rms, mean_fit, sigma_fit );
    MakeTGraphs(gr_Mean_apvchan_GEM[apv], gr_RMS_apvchan_GEM[apv], gr_MeanFit_apvchan_GEM[apv], gr_SigmaFit_apvchan_GEM[apv], mean, rms, mean_fit, sigma_fit);
    c2_gem->Clear();
    c2_gem->Divide(2,2);
    c2_gem->cd(1);
    gr_Mean_apvchan_GEM[apv]->SetMarkerStyle(20);
    gr_Mean_apvchan_GEM[apv]->SetTitle("; APV Channel # ; Mean of ADC");
    gr_Mean_apvchan_GEM[apv]->SetMarkerSize(0.5);
    gr_Mean_apvchan_GEM[apv]->SetLineWidth(1);
    gr_Mean_apvchan_GEM[apv]->SetMarkerColor(4);
    gr_Mean_apvchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_Mean_apvchan_GEM[apv]->Draw("APl");
    c2_gem->cd(2);
    gr_RMS_apvchan_GEM[apv]->SetMarkerStyle(20);
    gr_RMS_apvchan_GEM[apv]->SetTitle("; APV Channel # ; RMS of ADC");
    gr_RMS_apvchan_GEM[apv]->SetMarkerSize(0.5);
    gr_RMS_apvchan_GEM[apv]->SetLineWidth(1);
    gr_RMS_apvchan_GEM[apv]->SetMarkerColor(4);
    gr_RMS_apvchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_RMS_apvchan_GEM[apv]->Draw("APl");
    c2_gem->cd(3);
    gr_MeanFit_apvchan_GEM[apv]->SetMarkerStyle(20);
    gr_MeanFit_apvchan_GEM[apv]->SetTitle("; APV Channel # ; Mean of ADC");
    gr_MeanFit_apvchan_GEM[apv]->SetMarkerSize(0.5);
    gr_MeanFit_apvchan_GEM[apv]->SetLineWidth(1);
    gr_MeanFit_apvchan_GEM[apv]->SetMarkerColor(4);
    gr_MeanFit_apvchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_MeanFit_apvchan_GEM[apv]->Draw("APl");
    c2_gem->cd(4);
    gr_SigmaFit_apvchan_GEM[apv]->SetMarkerStyle(20);
    gr_SigmaFit_apvchan_GEM[apv]->SetTitle("; APV Channel # ; Sigma of ADC");
    gr_SigmaFit_apvchan_GEM[apv]->SetMarkerSize(0.5);
    gr_SigmaFit_apvchan_GEM[apv]->SetLineWidth(1);
    gr_SigmaFit_apvchan_GEM[apv]->SetMarkerColor(4);
    gr_SigmaFit_apvchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_SigmaFit_apvchan_GEM[apv]->Draw("APl");
    TCanvasPrint(c2_gem, TString("Figs/GEM_meansigma_apvchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
  //  c2_gem->Print(Form("Figs/GEM_meansigma_apvchan_apv_%d_run_%d.pdf", apv, run));
  //  c2_gem->Print(Form("Figs/GEM_meansigma_apvchan_apv_%d_run_%d.png", apv, run));
  //  c2_gem->Print(Form("Figs/GEM_meansigma_apvchan_apv_%d_run_%d.root", apv, run));

    mean.clear();
    rms.clear();
    mean_fit.clear();
    sigma_fit.clear();
    CalculateMeanAndSigma(h2_GEM_ADCaver_stripchan_apvslot[apv], mean, rms, mean_fit, sigma_fit );
    MakeTGraphs(gr_Mean_stripchan_GEM[apv], gr_RMS_stripchan_GEM[apv], gr_MeanFit_stripchan_GEM[apv], gr_SigmaFit_stripchan_GEM[apv], mean, rms, mean_fit, sigma_fit);
    WriteToFile(Form("PedFiles/PedestalGEM_apv%i_stripchannel__%d", apv, run), mean, rms, mean_fit, sigma_fit);
    c3_gem->Clear();
    c3_gem->Divide(2,2);
    c3_gem->cd(1);
    gr_Mean_stripchan_GEM[apv]->SetMarkerStyle(20);
    gr_Mean_stripchan_GEM[apv]->SetTitle("; Strip Channel # ; Mean of ADC");
    gr_Mean_stripchan_GEM[apv]->SetMarkerSize(0.5);
    gr_Mean_stripchan_GEM[apv]->SetLineWidth(1);
    gr_Mean_stripchan_GEM[apv]->SetMarkerColor(4);
    gr_Mean_stripchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_Mean_stripchan_GEM[apv]->Draw("APl");
    c3_gem->cd(2);
    gr_RMS_stripchan_GEM[apv]->SetMarkerStyle(20);
    gr_RMS_stripchan_GEM[apv]->SetTitle("; Strip Channel # ; RMS of ADC");
    gr_RMS_stripchan_GEM[apv]->SetMarkerSize(0.5);
    gr_RMS_stripchan_GEM[apv]->SetLineWidth(1);
    gr_RMS_stripchan_GEM[apv]->SetMarkerColor(4);
    gr_RMS_stripchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_RMS_stripchan_GEM[apv]->Draw("APl");
    c3_gem->cd(3);
    gr_MeanFit_stripchan_GEM[apv]->SetMarkerStyle(20);
    gr_MeanFit_stripchan_GEM[apv]->SetTitle("; Strip Channel # ; Mean of ADC");
    gr_MeanFit_stripchan_GEM[apv]->SetMarkerSize(0.5);
    gr_MeanFit_stripchan_GEM[apv]->SetLineWidth(1);
    gr_MeanFit_stripchan_GEM[apv]->SetMarkerColor(4);
    gr_MeanFit_stripchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_MeanFit_stripchan_GEM[apv]->Draw("APl");
    c3_gem->cd(4);
    gr_SigmaFit_stripchan_GEM[apv]->SetMarkerStyle(20);
    gr_SigmaFit_stripchan_GEM[apv]->SetTitle("; Strip Channel # ; Sigma of ADC");
    gr_SigmaFit_stripchan_GEM[apv]->SetMarkerSize(0.5);
    gr_SigmaFit_stripchan_GEM[apv]->SetLineWidth(1);
    gr_SigmaFit_stripchan_GEM[apv]->SetMarkerColor(4);
    gr_SigmaFit_stripchan_GEM[apv]->GetYaxis()->SetTitleOffset(0.5);
    gr_SigmaFit_stripchan_GEM[apv]->Draw("APl");
    TCanvasPrint(c3_gem, TString("Figs/GEM_meansigma_stripchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
  //  c3_gem->Print(Form("Figs/GEM_meansigma_stripchan_apv_%d_run_%d.pdf", apv, run));
  //  c3_gem->Print(Form("Figs/GEM_meansigma_stripchan_apv_%d_run_%d.png", apv, run));
  //  c3_gem->Print(Form("Figs/GEM_meansigma_stripchan_apv_%d_run_%d.root", apv, run));

    canv_single->Clear();
    canv_single->cd();
    gr_Mean_stripchan_GEM[apv]->Draw("APl");
    TCanvasPrint(canv_single, TString("Figs/SingleAPVs/GEM_mean_stripchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
    canv_single->Clear();
    canv_single->cd();
    gr_RMS_stripchan_GEM[apv]->Draw("APl");
    TCanvasPrint(canv_single, TString("Figs/SingleAPVs/GEM_rms_stripchan_apv_"+to_string(apv)+"_run_"+to_string(run) ));
    canv_single->Clear();

    file_out->cd();
    gr_Mean_apvchan_GEM[apv]->Write(Form("GEM_Mean_apvchan_apv_%i",apv));
    gr_RMS_apvchan_GEM[apv]->Write(Form("GEM_RMS_apvchan_apv_%i",apv));
    gr_Mean_stripchan_GEM[apv]->Write(Form("GEM_Mean_stripchan_apv_%i",apv));
    gr_RMS_stripchan_GEM[apv]->Write(Form("GEM_RMS_stripchan_apv_%i",apv));
  }


    return 0;
}

void DrawHybridLimits(TGraph* gr, double max){
    //double max = gr->GetMaximum();
    //double max = 120.;
    //cout<<"MAXIMUM OF THE GRAPH IS "<<max<<endl;
    TLine *line1 = new TLine();
    line1->SetLineStyle(9);
    line1->SetLineColor(2);
    line1->SetLineWidth(2);
    line1->DrawLine(64, 0., 64., max);
    line1->DrawLine(1064, 0., 1064., max);
    const int n_Hybrid_Side = 6;
    const int nchHybris = 128;
    for( int i = 0; i < n_Hybrid_Side - 1; i++ ){
        line1->DrawLine(64 + nchHybris*i, 0, 64 + nchHybris*i, max);
        line1->DrawLine(1064 + nchHybris*i, 0, 1064 + nchHybris*i, max);
    }
}

void WriteToFileAll(TString filename, std::map< int, double > &mean, std::map< int, double > &rms, std::map< int, double > &mean_fit, std::map< int, double > &sigma_fit ){
  ofstream out_ped(filename);
  out_ped << setw(5) << "channelid" << " " << setw(5) << "mean" << " " << setw(5) << "mean_fit" << " " << setw(5) << "rms"  << " " << setw(5) << "sigma_fit" << endl;
  //loop only over mean but addressing the other maps would be similar.
  for (const auto& it : mean) {
    out_ped << setw(5) << it.first << " " << setw(5) << mean[it.first] << " " << setw(5) << mean_fit[it.first] << " " << setw(5) << rms[it.first] << " " << setw(5) << sigma_fit[it.first] << endl;
  }
}

void WriteToFile(TString filename, std::map< int, double > &mean, std::map< int, double > &rms, std::map< int, double > &mean_fit, std::map< int, double > &sigma_fit ){
  ofstream out_ped(filename);
  out_ped << setw(5) << "channelid" << " " << setw(5) << "mean" << " " << "rms"  << endl;
  //loop only over mean but addressing the other maps would be similar.
  for (const auto& it : mean) {
    out_ped << setw(5) << it.first << " " << setw(5) << mean[it.first] << " " << setw(5) << rms[it.first] << endl;
  }
}


void CalculateMeanAndSigma(TH2D* inputhisto, std::map< int, double > &mean, std::map< int, double > &rms, std::map< int, double > &mean_fit, std::map< int, double > &sigma_fit ) {

  TF1 *f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", 1000, 3500);
  for (int ibin = 0; ibin < inputhisto->GetNbinsX(); ibin++) {
      TH1D *h_tmp = (TH1D*) inputhisto->ProjectionY(Form("h_ADC_ch_%d", ibin + 1), ibin + 1, ibin + 1);

      if (h_tmp->GetEntries() == 0) {
          continue;
      }

      int det_chan = int(inputhisto->GetXaxis()->GetBinCenter(ibin + 1));

      mean[det_chan] = h_tmp->GetMean();
      rms[det_chan] = h_tmp->GetRMS();

      f_Gaus->SetParameters(h_tmp->GetMaximum(), mean[det_chan], rms[det_chan]);
      h_tmp->Fit(f_Gaus, "MeV", "", mean[det_chan] - 4 * rms[det_chan], mean[det_chan] + 4 * rms[det_chan]);
      mean_fit[det_chan] = f_Gaus->GetParameter(1);
      sigma_fit[det_chan] = f_Gaus->GetParameter(2);
      delete h_tmp;
  }
  delete f_Gaus;
}

void MakeTGraphs(TGraph *gr_Mean, TGraph *gr_RMS, TGraph *gr_MeanFit, TGraph *gr_SigmaFit, std::map< int, double > &mean, std::map< int, double > &rms, std::map< int, double > &mean_fit, std::map< int, double > &sigma_fit ) {

    int index = 0;
    for (const auto& it : mean) {
      gr_Mean->SetPoint(index, it.first, it.second);
      index++;
    }
    index = 0;
    for (const auto& it : rms) {
      gr_RMS->SetPoint(index, it.first, it.second);
      index++;
    }
    index = 0;
    for (const auto& it : mean_fit) {
      gr_MeanFit->SetPoint(index, it.first, it.second);
      index++;
    }
    index = 0;
    for (const auto& it : sigma_fit) {
      gr_SigmaFit->SetPoint(index, it.first, it.second);
      index++;
    }

}

void TCanvasPrint(TCanvas *canvasinput, TString filename) {

    canvasinput->Print(filename+".pdf");
    canvasinput->Print(filename+".png");
    canvasinput->Print(filename+".root");

}
