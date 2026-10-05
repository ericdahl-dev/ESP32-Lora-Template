# ESP32 Template Web Flasher

A web-based ESP32 firmware flasher for the ESP32 Modular Device Template.

## 🚀 Features

- **🌐 Web-based ESP32 Flasher**: Flash ESP32 devices directly from your browser
- **📦 Automatic Release Detection**: Automatically finds and downloads latest firmware from GitHub releases
- **📁 Manual Upload Support**: Upload firmware files directly from your computer
- **📱 Responsive Design**: Works on desktop and mobile devices
- **🔧 Professional Build System**: Uses npm and Vite for reliable builds

## 🛠️ Development

### Prerequisites

- Node.js 18+
- npm or yarn

### Setup

1. **Install dependencies**:
   ```bash
   cd web-flasher
   npm install
   ```

2. **Start development server**:
   ```bash
   npm run dev
   ```

3. **Build for production**:
   ```bash
   npm run build
   ```

4. **Preview production build**:
   ```bash
   npm run preview
   ```

## 📁 Project Structure

```
web-flasher/
├── src/
│   └── main.js          # Main JavaScript application
├── index.html            # Web flasher HTML page
├── package.json          # npm dependencies and scripts
├── vite.config.js        # Vite build configuration
├── .eslintrc.js          # ESLint code quality rules
└── .gitignore            # Git ignore rules
```

## 🔧 Build Process

The web flasher uses:
- **Vite**: Fast, modern build tool
- **ESLint**: Code quality enforcement
- **esp-web-flasher**: ESP32 flashing library

## 🚀 Deployment

This web flasher is automatically built and deployed to GitHub Pages when changes are pushed to the main branch.

## 📱 Browser Support

- Chrome/Edge (Web Serial API support)
- Firefox (limited Web Serial API support)
- Safari (no Web Serial API support)

## 🔌 Hardware Requirements

- ESP32 device (any supported board)
- USB cable for serial connection
- Modern browser with Web Serial API support
