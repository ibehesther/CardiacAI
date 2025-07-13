import React, { createContext, useContext, useState } from "react";
import { getDeviceMetadata, loginUser } from "../services/auth";

interface AuthContextType {
	isAuthenticated: boolean;
	userRole?: string;
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
	const [userRole, setUserRole] = useState("");

	const verifyToken = async () => {
		console.log("Verifying token...");
		const token = localStorage.getItem("cardiac_ai_access_token") || "";
		const deviceId = localStorage.getItem("cardiac_ai_device_id") || "";
		const role = localStorage.getItem("cardiac_ai_user_role") || "";

		if (!token || !deviceId) {
			setIsAuthenticated(false);
			setLoading(false);
			return;
		}
		try {
			const deviceMetadata = await getDeviceMetadata(deviceId, token);
			if (
				!deviceMetadata[0] ||
				!deviceMetadata[0].device_id ||
				!deviceMetadata[0].session_id
			) {
				setIsAuthenticated(false);
				return;
			} else {
				setDeviceMetadata(deviceMetadata[deviceMetadata.length - 1]);
				setUserRole(role || "user");
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
			localStorage.setItem("cardiac_ai_user_role", response.role);

			setIsAuthenticated(true);
			setUserRole(response.role || "user");

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
			value={{
				isAuthenticated,
				loading,
				deviceMetadata,
				userRole,
				verifyToken,
				login,
				logout,
			}}
		>
			{children}
		</AuthContext.Provider>
	);
};

export const useAuth = () => useContext(AuthContext)!;



// {
//     "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJkZXZpY2VfaWQiOiJjYXJkaWFjYWktMTIzIiwiZXhwIjoxNzUyMzQyOTg2LCJyb2xlIjoiYWRtaW4ifQ.OQ-T9xAcAuqey61vaKwu6nzRLE_8uPzHYTOCUX-DJHk",
// 		 "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJkZXZpY2VfaWQiOiJjYXJkaWFjYWktMTIzIiwiZXhwIjoxNzUyMzQyOTg2LCJyb2xlIjoiYWRtaW4ifQ.OQ-T9xAcAuqey61vaKwu6nzRLE_8uPzHYTOCUX-DJHk
//     "token_type": "bearer",
//     "role": "admin"
// }