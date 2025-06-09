import React, { createContext, useContext, useEffect, useState } from "react";
import { getDeviceMetadata, loginUser } from "../services/auth";

interface AuthContextType {
	isAuthenticated: boolean;
  loading: boolean;
  deviceMetadata: any;
	verifyToken: () => void;
	login: (username: string, password: string) => Promise<void>;
	logout: () => void;
}

const AuthContext = createContext<AuthContextType | null>(null);

export const AuthProvider: React.FC<{ children: React.ReactNode }> = ({
	children,
}) => {
	const [isAuthenticated, setIsAuthenticated] = useState<boolean>(false);
  const [deviceMetadata, setDeviceMetadata] = useState<any>(null);
  const [loading, setLoading] = useState(true);

  
	const verifyToken = async () => {
    console.log('Verifying token...');
    const token = localStorage.getItem("cardiac_ai_access_token") || "";
		const deviceId = localStorage.getItem("cardiac_ai_device_id") || "";

		if (!token || !deviceId) {
			setIsAuthenticated(false);
      setLoading(false);
			return;
		}
    try {

      const deviceMetadata = await getDeviceMetadata(deviceId, token);
      console.log("deviceMetadata: ", deviceMetadata);
      if (
        !deviceMetadata[0] ||
        !deviceMetadata[0].device_id ||
        !deviceMetadata[0].session_id
      ) {
        setIsAuthenticated(false);
        return;
      } else {
        setDeviceMetadata(deviceMetadata[0]);
        setIsAuthenticated(true);
        return;
      }
    } catch (err) {
			console.error("Token verification failed", err);
			setIsAuthenticated(false);
		} finally {
			setLoading(false);
		}
	};

	const login = async (username: string, password: string) => {
		try {
			const response = await loginUser(username, password);
			localStorage.setItem("cardiac_ai_device_id", username);
			localStorage.setItem("cardiac_ai_access_token", response.access_token);
			setIsAuthenticated(true);
		} catch (error) {
			console.error("Login failed:", error);
			throw new Error("Login failed. Please check your credentials.");
		}
	};

 
	const logout = () => {
		localStorage.removeItem("cardiac_ai_access_token");
		setIsAuthenticated(false);
	};


	return (
		<AuthContext.Provider
			value={{ isAuthenticated, loading, deviceMetadata, verifyToken, login, logout }}
		>
			{children}
		</AuthContext.Provider>
	);
};

export const useAuth = () => useContext(AuthContext)!;
