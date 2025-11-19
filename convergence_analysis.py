import OpenEXR
import Imath
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

class PathTracerConvergenceAnalyzer:
    def __init__(self):
        self.scenes = {
            'cb_basic': {
                'files': {
                    2: 'cb_basic_2.exr',
                    8: 'cb_basic_8.exr',
                    32: 'cb_basic_32.exr',
                    128: 'cb_basic_128.exr',
                    512: 'cb_basic_512.exr'
                },
                'times': {
                    2: 20.3116,
                    8: 81.2371,
                    32: 326.0044,
                    128: 1337.6823,
                    512: 5171.893
                },
                'description': 'Cornell Box - Esferas difusas'
            },
            'cb_e2': {
                'files': {
                    2: 'cb_e2_2.exr',
                    8: 'cb_e2_8.exr',
                    32: 'cb_e2_32.exr',
                    128: 'cb_e2_128.exr',
                    512: 'cb_e2_512.exr'
                },
                'times': {
                    2: 20.9473,
                    8: 81.6962,
                    32: 326.2717,
                    128: 1312.1938,
                    512: 5223.3374
                },
                'description': 'Cornell Box - Esfera plástica y dieléctrica'
            }
        }
        
        self.base_path = Path('images_guion_p1')
        
    def read_exr(self, filepath):
        """Lee una imagen EXR y devuelve un array numpy RGB."""
        exr_file = OpenEXR.InputFile(str(filepath))
        header = exr_file.header()
        
        dw = header['dataWindow']
        width = dw.max.x - dw.min.x + 1
        height = dw.max.y - dw.min.y + 1
        
        FLOAT = Imath.PixelType(Imath.PixelType.FLOAT)
        
        r_str = exr_file.channel('R', FLOAT)
        g_str = exr_file.channel('G', FLOAT)
        b_str = exr_file.channel('B', FLOAT)
        
        r = np.frombuffer(r_str, dtype=np.float32).reshape(height, width)
        g = np.frombuffer(g_str, dtype=np.float32).reshape(height, width)
        b = np.frombuffer(b_str, dtype=np.float32).reshape(height, width)
        
        img = np.stack([r, g, b], axis=2)
        return img
    
    def compute_mse(self, img1, img2):
        """Calcula el Mean Squared Error entre dos imágenes."""
        return np.mean((img1 - img2) ** 2)
    
    def compute_rmse(self, img1, img2):
        """Calcula el Root Mean Squared Error."""
        return np.sqrt(self.compute_mse(img1, img2))
    
    def analyze_convergence_rate(self, errors, spp_values):
        """
        Analiza la tasa de convergencia.
        Para Monte Carlo, esperamos error ∝ 1/√N
        """
        log_spp = np.log(spp_values)
        log_errors = np.log(errors)
        
        coeffs = np.polyfit(log_spp, log_errors, 1)
        slope = coeffs[0]
        
        return slope, coeffs
    
    def analyze_scene(self, scene_name, scene_data):
        """Analiza la convergencia para una escena específica."""
        print(f"\n{'='*70}")
        print(f"Análisis de Convergencia: {scene_name}")
        print(f"Descripción: {scene_data['description']}")
        print(f"{'='*70}\n")
        
        # Cargar todas las imágenes
        images = {}
        spp_list = sorted(scene_data['files'].keys())
        
        for spp in spp_list:
            filename = self.base_path / scene_data['files'][spp]
            if filename.exists():
                images[spp] = self.read_exr(filename)
                print(f"✓ Cargada imagen: {scene_data['files'][spp]}")
            else:
                print(f"✗ No encontrada: {scene_data['files'][spp]}")
        
        if len(images) < 2:
            print("Error: Se necesitan al menos 2 imágenes para el análisis.")
            return None
        
        # Usar la imagen de mayor SPP como referencia
        reference_spp = max(images.keys())
        reference_img = images[reference_spp]
        print(f"\n→ Usando {reference_spp} SPP como referencia\n")
        
        # Calcular métricas de error
        results = {
            'spp': [],
            'mse': [],
            'rmse': [],
            'time': [],
            'time_per_spp': [],
            'error_reduction': []  # Factor de reducción del error entre niveles
        }
        
        print(f"{'SPP':<8} {'MSE':<16} {'RMSE':<16} {'Tiempo (s)':<14} {'Tiempo/SPP':<14}")
        print("-" * 72)
        
        for spp in sorted(images.keys()):
            if spp == reference_spp:
                continue
                
            img = images[spp]
            mse = self.compute_mse(img, reference_img)
            rmse = self.compute_rmse(img, reference_img)
            
            results['spp'].append(spp)
            results['mse'].append(mse)
            results['rmse'].append(rmse)
            
            if spp in scene_data['times']:
                time = scene_data['times'][spp]
                time_per_spp = time / spp
                
                results['time'].append(time)
                results['time_per_spp'].append(time_per_spp)
                
                print(f"{spp:<8} {mse:<16.10f} {rmse:<16.10f} {time:<14.4f} {time_per_spp:<14.8f}")
            else:
                results['time'].append(0)
                results['time_per_spp'].append(0)
                print(f"{spp:<8} {mse:<16.10f} {rmse:<16.10f} {'N/A':<14} {'N/A':<14}")
        
        # Análisis de convergencia
        print(f"\n{'─'*70}")
        print("ANÁLISIS DE CONVERGENCIA")
        print(f"{'─'*70}")
        
        if len(results['rmse']) > 1:
            slope, coeffs = self.analyze_convergence_rate(results['rmse'], results['spp'])
            print(f"\n→ Tasa de convergencia (pendiente en log-log): {slope:.4f}")
            print(f"  • Monte Carlo ideal: -0.5")
            print(f"  • Obtenido: {slope:.4f}")
            
            if abs(slope + 0.5) < 0.1:
                print("  ✓ Convergencia cercana a la ideal de Monte Carlo")
            elif slope > -0.3:
                print("  ⚠ Convergencia más lenta de lo esperado")
            else:
                print("  ✓ Convergencia mejor que Monte Carlo estándar")
            
            # Calcular factor de mejora entre niveles de SPP
            print(f"\n→ Factores de reducción del error (RMSE):")
            for i in range(1, len(results['spp'])):
                spp_prev = results['spp'][i-1]
                spp_curr = results['spp'][i]
                rmse_prev = results['rmse'][i-1]
                rmse_curr = results['rmse'][i]
                
                factor_spp = spp_curr / spp_prev
                factor_error = rmse_prev / rmse_curr
                theoretical_factor = np.sqrt(factor_spp)
                
                results['error_reduction'].append(factor_error)
                
                print(f"  • {spp_prev} → {spp_curr} SPP: error reduce {factor_error:.2f}x "
                      f"(teórico: {theoretical_factor:.2f}x)")
        
        # Análisis de eficiencia temporal
        if any(results['time']):
            print(f"\n{'─'*70}")
            print("ANÁLISIS DE EFICIENCIA TEMPORAL")
            print(f"{'─'*70}")
            print(f"\n{'SPP':<8} {'Tiempo total':<15} {'Tiempo/SPP':<15} {'Escalabilidad':<15}")
            print("-" * 55)
            
            base_time_per_spp = results['time_per_spp'][0] if results['time_per_spp'][0] > 0 else 1
            for i, spp in enumerate(results['spp']):
                if results['time'][i] > 0:
                    time = results['time'][i]
                    time_per_spp = results['time_per_spp'][i]
                    scaling = time_per_spp / base_time_per_spp
                    print(f"{spp:<8} {time:<15.4f} {time_per_spp:<15.8f} {scaling:<15.4f}x")
            
            # Calcular tiempo estimado para alcanzar menor error
            print(f"\n→ Proyección temporal:")
            if len(results['time']) > 1:
                max_spp = results['spp'][-1]
                max_time = results['time'][-1]
                next_spp = max_spp * 2
                estimated_time = max_time * 2
                
                print(f"  • Para alcanzar {next_spp} SPP: ~{estimated_time:.2f} segundos")
                print(f"  • Mejora de error esperada: {results['rmse'][-1] / np.sqrt(2):.6f} RMSE")
        
        # Generar gráficas
        self.plot_convergence(results, scene_name, scene_data['description'])
        
        return results
    
    def plot_convergence(self, results, scene_name, description):
        """Genera gráficas de convergencia."""
        fig = plt.figure(figsize=(14, 10))
        fig.suptitle(f'Análisis de Convergencia - {scene_name}\n{description}', 
                     fontsize=16, fontweight='bold')
        
        spp = np.array(results['spp'])
        
        # Crear grid de subplots
        gs = fig.add_gridspec(3, 2, hspace=0.3, wspace=0.3)
        
        # 1. RMSE vs SPP (escala log-log) - Principal
        ax1 = fig.add_subplot(gs[0:2, 0])
        ax1.loglog(spp, results['rmse'], 'o-', linewidth=2, markersize=10, 
                   label='RMSE medido', color='#2E86AB')
        
        # Línea teórica 1/√N
        theoretical = results['rmse'][0] * np.sqrt(spp[0] / spp)
        ax1.loglog(spp, theoretical, '--', linewidth=2, alpha=0.7, 
                   label='1/√N teórico', color='#A23B72')
        
        ax1.set_xlabel('Samples per Pixel (SPP)', fontsize=12, fontweight='bold')
        ax1.set_ylabel('RMSE', fontsize=12, fontweight='bold')
        ax1.set_title('Convergencia del Error (escala log-log)', fontsize=14, fontweight='bold')
        ax1.grid(True, alpha=0.3, linestyle='--')
        ax1.legend(fontsize=11)
        
        # Añadir anotaciones con valores
        for i, (x, y) in enumerate(zip(spp, results['rmse'])):
            ax1.annotate(f'{y:.4f}', (x, y), textcoords="offset points", 
                        xytext=(0,10), ha='center', fontsize=8, alpha=0.7)
        
        # 2. MSE vs SPP
        ax2 = fig.add_subplot(gs[0:2, 1])
        ax2.loglog(spp, results['mse'], 's-', linewidth=2, markersize=10, color='#F18F01')
        ax2.set_xlabel('SPP', fontsize=12, fontweight='bold')
        ax2.set_ylabel('MSE', fontsize=12, fontweight='bold')
        ax2.set_title('Error Cuadrático Medio (escala log-log)', fontsize=14, fontweight='bold')
        ax2.grid(True, alpha=0.3, linestyle='--')
        
        # Añadir anotaciones con valores
        for i, (x, y) in enumerate(zip(spp, results['mse'])):
            ax2.annotate(f'{y:.6f}', (x, y), textcoords="offset points", 
                        xytext=(0,10), ha='center', fontsize=8, alpha=0.7)
        
        # 3. Tiempo de renderizado vs SPP
        if any(results['time']):
            ax3 = fig.add_subplot(gs[2, 0])
            valid_times = [(s, t) for s, t in zip(spp, results['time']) if t > 0]
            if valid_times:
                valid_spp, valid_time = zip(*valid_times)
                ax3.plot(valid_spp, valid_time, 'o-', linewidth=2, markersize=8, color='#C73E1D')
                ax3.set_xlabel('SPP', fontsize=11, fontweight='bold')
                ax3.set_ylabel('Tiempo (s)', fontsize=11, fontweight='bold')
                ax3.set_title('Tiempo de Renderizado', fontsize=12, fontweight='bold')
                ax3.grid(True, alpha=0.3, linestyle='--')
        
        # 4. Eficiencia: Error vs Tiempo
        if any(results['time']):
            ax4 = fig.add_subplot(gs[2, 1])
            valid_data = [(t, r) for t, r in zip(results['time'], results['rmse']) if t > 0]
            if valid_data:
                valid_time, valid_rmse = zip(*valid_data)
                ax4.loglog(valid_time, valid_rmse, 'o-', linewidth=2, markersize=8, color='#9D4EDD')
                ax4.set_xlabel('Tiempo (s)', fontsize=11, fontweight='bold')
                ax4.set_ylabel('RMSE', fontsize=11, fontweight='bold')
                ax4.set_title('Eficiencia: Error vs Tiempo', fontsize=12, fontweight='bold')
                ax4.grid(True, alpha=0.3, linestyle='--')
        
        output_file = self.base_path / f'convergence_analysis_{scene_name}.png'
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"\n✓ Gráfica guardada: {output_file}")
        plt.close()
    
    def run_full_analysis(self):
        """Ejecuta el análisis completo para todas las escenas."""
        print("\n" + "="*70)
        print("ANÁLISIS DE CONVERGENCIA DE PATH TRACER - MSE/RMSE")
        print("="*70)
        
        all_results = {}
        for scene_name, scene_data in self.scenes.items():
            results = self.analyze_scene(scene_name, scene_data)
            if results:
                all_results[scene_name] = results
        
        print("\n" + "="*70)
        print("ANÁLISIS COMPLETADO EXITOSAMENTE")
        print("="*70)
        
        return all_results


if __name__ == "__main__":
    analyzer = PathTracerConvergenceAnalyzer()
    results = analyzer.run_full_analysis()