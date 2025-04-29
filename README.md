# 💬 C++ Multi-Client Chat Application

**Personal Project** · **April 2025**

A multithreaded chat server and client built in **C++** using **socket programming** (Winsock), supporting real-time messaging, group chats, user listings, and an extensible command-line interface.

---

## 🚀 Features

- 🔐 **Private Messaging** – Send direct messages using `/msg <user> <message>`
- 👥 **Group Chat Support** – Join or create groups, and chat with multiple users
- 📋 **Active User List** – View currently connected users with `/users`
- ⌛ **Timestamped Messages** – Messages are displayed with timestamps
- 🧱 **Command Interface** – Built-in commands for easy interaction

### 📜 Supported Commands
| Command                  | Description                            |
|--------------------------|----------------------------------------|
| `/msg <user> <message>`  | Send a private message                 |
| `/users`                 | List all online users                  |
| `/create <group>`        | Create a new group chat                |
| `/join <group>`          | Join an existing group                 |
| `/gmsg <message>`        | Send a message to your current group   |
| `/help`                  | Display all available commands         |

---

## 🛠️ Technologies Used

- **C++**
- **Winsock (Windows Sockets API)**
- **TCP/IP Networking**
- **Multithreading**
- **STL (Standard Template Library)**

---

## 🧪 How to Run

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/cpp-multiclient-chat.git
   ```
2. **Compile both server and client:**
   - On Windows, use a compiler like MSVC or MinGW.
3. **Run the Server:**
   ```bash
   ./server
   ```
4. **Run the Client:**
   ```bash
   ./client <server-ip-address>
   ```

> ✅ Ensure all clients are on the same network or that the server is reachable via IP.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

## 👨‍💻 Author

**ROHIT**  
[GitHub Profile](https://github.com/Pralayraj-KilvishDal)

