import logo from '../assets/images/logo.png'
import '../styles/header.css'
import { Link } from "react-router-dom"

const Header = () => {
    return (
        <header className="header">
            <div className="logo-div">
                <Link to="/" className="logo">
                    <img src={logo || "/placeholder.svg"} alt="logo"></img>
                </Link>
            </div>
            <div className="auth-buttons">
                <Link to="/login" className="login-button">
                    LOG IN
                </Link>
            </div>
        </header>
    )
}

export default Header;