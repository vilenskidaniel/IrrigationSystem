// script.js
// Этот файл отвечает за:
// - Обработку звуков при нажатии кнопок
// - Переходы между страницами (fade out -> spinner -> load new page)
// - Показ и скрытие модальных окон, toast, tooltip
// - Смена темы, смена языка (предполагается AJAX подгрузка словарей)
// - Реакция на удержание элементов для подсказок
// - Воспроизведение анимаций и звуков при различных событиях
// Подробные комментарии внутри кода.

document.addEventListener('DOMContentLoaded', () => {
  // Инициализация после загрузки DOM
  // Можно отыграть startup_sound.mp3, например, при загрузке главной страницы
  if (window.location.pathname === "/index.html" || window.location.pathname === "/") {
    playSound('startup_sound.mp3');
  }

  // Добавим слушатель глобального нажатия для воспроизведения звука при кликах на активные кнопки
  // Активные кнопки сами вызывают playSound при onmousedown
  // Блокируем скролл pinch-zoom (мета-тег user-scalable=no уже сделан)
  
  // Логика удержания для подсказок:
  setupTooltips();

  // При смене языка/темы или клике на переходы – будут специальные функции.
});

// ============ Функции звука ============
function playSound(filename) {
  // Создаём аудио-объект и воспроизводим
  const audio = new Audio('/audio/' + filename);
  audio.play().catch(e=>console.log('Audio play error:', e));
}

// ============ Переходы между страницами ============
function navigateTo(url) {
  // При нажатии на кнопку перехода:
  // 1) Скрыть контент (fade out)
  // 2) Показать spinner
  // 3) Через небольшой таймер перейти по URL

  fadeOutContent(() => {
    showSpinner();
    setTimeout(() => {
      window.location.href = url;
    }, 500); 
  });
}

function fadeOutContent(callback) {
  const pageContainer = document.querySelector('.page-container');
  if (pageContainer) {
    pageContainer.style.transition = 'opacity 0.5s';
    pageContainer.style.opacity = '0';
    setTimeout(() => {
      if (callback) callback();
    }, 500);
  } else {
    if (callback) callback();
  }
}

function showSpinner() {
  // Показать анимацию connecting_spinner.webp в правом нижнем углу
  let spinner = document.createElement('div');
  spinner.className = 'connecting-spinner';
  spinner.innerHTML = `<div class="spinner-frame">
                          <img src="/animations/connecting_spinner.webp" alt="Loading...">
                       </div>`;
  document.body.appendChild(spinner);
}

// ============ Модальные окна ============
function showModal(message, type="info", buttons=[{text:"OK",callback:()=>closeModal()}]) {
  const modal = document.getElementById('commonModal');
  if (!modal) return;
  modal.style.display = 'flex';
  const msgEl = modal.querySelector('#modal-message');
  msgEl.textContent = message;

  const iconEl = modal.querySelector('#modal-icon');
  if (type === "question") {
    iconEl.src = "/images/icons/question_icon.png";
  } else if (type === "warning") {
    iconEl.src = "/images/icons/warning_icon.png";
  } else if (type === "notification") {
    iconEl.src = "/images/icons/notification_icon.png";
  } else {
    iconEl.src = "/images/icons/notification_icon.png";
  }

  const btnContainer = modal.querySelector('#modal-buttons');
  btnContainer.innerHTML = '';
  buttons.forEach(b => {
    const btn = document.createElement('button');
    btn.textContent = b.text;
    btn.onclick = () => { 
      playSound('button_click.mp3');
      b.callback(); 
    };
    btnContainer.appendChild(btn);
  });
}

function closeModal() {
  const modal = document.getElementById('commonModal');
  if (modal) modal.style.display='none';
}

// ============ Toast ============
function showToast(message) {
  const toast = document.querySelector('.toast');
  if (!toast) return;
  toast.textContent = message;
  toast.style.display = 'block';
  setTimeout(()=> { toast.style.display='none'; },3000);
}

// ============ Подсказки (Tooltips) ============
function setupTooltips() {
  // Идея: Элементы, у которых есть data-tooltip="текст"
  // При удержании более 1сек показать tooltip
  let pressTimer;
  document.body.addEventListener('touchstart', (e)=>{
    let target = e.target.closest('[data-tooltip]');
    if (target) {
      pressTimer = setTimeout(()=>{
        showTooltip(target, target.getAttribute('data-tooltip'));
      },1000);
    }
  });
  document.body.addEventListener('touchend', (e)=>{
    clearTimeout(pressTimer);
    hideTooltip();
  });
}

function showTooltip(element, text) {
  let tooltip = document.createElement('div');
  tooltip.className = 'tooltip';
  tooltip.textContent = text;
  document.body.appendChild(tooltip);

  let rect = element.getBoundingClientRect();
  tooltip.style.left = (rect.left + rect.width/2 - tooltip.offsetWidth/2) + 'px';
  tooltip.style.top = (rect.top - tooltip.offsetHeight - 10) + 'px';
}

function hideTooltip() {
  let t = document.querySelector('.tooltip');
  if (t) t.remove();
}

// ============ Управление Turret (заглушка) ============
function sendControlCommand(command) {
  playSound('button_click.mp3');
  // Здесь можно послать AJAX запрос на RPi
  showToast("Команда отправлена: " + command);
}

// ============ Язык, Тема, Возврат ============
function goBack() {
  navigateTo('/index.html');
}

function toggleTheme() {
  // Переключение темы
  // Применяем мгновенно
  document.body.classList.toggle('dark-theme');
  document.body.classList.toggle('light-theme');
  playSound('button_click.mp3');
}

function changeLanguage(lang) {
  // AJAX для подгрузки словаря и обновления текста
  // Сейчас только звук и тост для примера
  playSound('button_click.mp3');
  showToast("Язык изменён на: " + lang);
  // В реальности: обновить все тексты на странице
}

// =========== Wi-Fi перенастройка =============
function reconfigureWiFi() {
  showModal("Перейти в режим настройки Wi-Fi?", "question", [
    {text:"Да", callback:()=>{playSound('notification.mp3'); /* отправить команду/перезагрузить*/ closeModal();}},
    {text:"Нет", callback:()=>closeModal()}
  ]);
}

// =========== Сброс =============
function resetAll() {
  showModal("Сбросить все настройки?", "question", [
    {text:"Да", callback:()=>{playSound('success_wattering.mp3'); /*Команда сброса*/ closeModal();}},
    {text:"Нет", callback:()=>closeModal()}
  ]);
}

// Дополнительные функции могут быть добавлены при необходимости.
